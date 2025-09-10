import os
from multiprocessing import Pool, cpu_count
import numpy as np
import pandas as pd
import argparse
import sys
import pickle
import datetime

import tensorflow as tf
from nnarchitecture import *
from sklearn.model_selection import train_test_split

from sklearn.preprocessing import StandardScaler
from sklearn.utils import class_weight
from keras.utils import to_categorical

import code  # code.interact(local=dict(globals(), **locals()))

def generate_model_nickname(model):
    layer_info = []
    total_params = model.count_params()

    for layer in model.layers:
        class_name = layer.__class__.__name__
        config = layer.get_config()

        if class_name == 'Dense':
            layer_info.append(f"D{config['units']}")
        elif class_name == 'Conv2D':
            layer_info.append(f"C{config['filters']}x{config['kernel_size'][0]}")
        elif class_name == 'MaxPooling2D':
            layer_info.append("MP")
        elif class_name == 'Dropout':
            layer_info.append(f"DO{int(config['rate'] * 100)}")
        elif class_name == 'Flatten':
            layer_info.append("F")
        elif class_name == 'BatchNormalization':
            layer_info.append("BN")
        # You can add more cases as needed

    # Join the layer summary and attach total params
    nickname = "_".join(layer_info)
    nickname += f"_P{total_params}"

    return nickname

# Training function for a given batch size
def train_model(train_ds, val_ds, n_steps, n_features, datafilename, bs, withgpu, class_weights, scaler, weight_strategy):

    ###########################################
    saveas = datafilename.split(
        "/")[-1].replace("ALL_ysequences_", "").replace(".parquet", "")
    print(f"\nTraining with batch_size={bs}")

    # Build a new model each time
    model = build_mlp(num_steps=4, num_features=6)
    model.summary()
    nickname = generate_model_nickname(model)
    x = datetime.datetime.now()
    s =x.strftime("%d-%m-%y_%H:%M")

    saveas = nickname+"_"+s+"_"+saveas

    # Train
    history = model.fit(
        train_ds,
        epochs=100,
        batch_size=bs,
        validation_data=val_ds,
        class_weight=class_weights,
        callbacks=[es, lr],
        verbose=1
    )

    pred = model.predict(val_ds, verbose=1)
    Y_pred_bool = np.argmax(pred, axis=1)

    # Evaluate performances and print
    performance = model.evaluate(val_ds)
    print(f'Test Loss: {performance[0]}, Test Accuracy: {performance[1]}')

    # Save the data
    scaler_filename = f"standard_scaler_{saveas}_bs_{bs}_cw_{weight_strategy}.pkl"
    with open(scaler_filename, 'wb') as f:
        pickle.dump(scaler, f)
    model.save(f'model_{saveas}_bs_{bs}_cw_{weight_strategy}.keras',  include_optimizer=False)

    file = open(f'plottable_{saveas}_bs_{bs}_cw_{weight_strategy}.pkl', 'wb')
    X_val, Y_val = dataset_to_numpy(val_ds)
    tobeDumped = [history, Y_val, Y_pred_bool]
    pickle.dump(tobeDumped, file)
    file.close()

    # Best validation accuracy
    best_val_acc = max(history.history['val_accuracy'])
    print(f"Best val_accuracy: {best_val_acc:.4f}")
    return {
        'batch_size': bs,
        'best_val_accuracy': best_val_acc,
        'epochs_ran': len(history.history['loss']),
        'pid': os.getpid()
    }


def spilt_scale_dataset(X_df, Y_df, n_steps, n_features, weight_strategy, batch_size=64):
    # Split, Scale and Shuffle
    # Split the data first
    x_train, x_val, y_train, y_val = train_test_split(X_df, Y_df, test_size=0.33, random_state=42)

    # Fit the scaler only on the training data
    scaler = StandardScaler()
    # fit_transform is like fit()+transform() and is faster
    x_train_scaled = scaler.fit_transform(x_train)

    # Use the fitted scaler to transform validation data
    x_val_scaled = scaler.transform(x_val)

    # restore X matrix/tensor shape for both train and validation
    X_train = np.array([row.reshape(n_steps, n_features)
                        for row in x_train_scaled])
    X_val = np.array([row.reshape(n_steps, n_features)
                      for row in x_val_scaled])

    # Make sure Y is a 1D NumPy array
    Y_train = y_train.to_numpy()
    Y_val = y_val.to_numpy()

    # Compute class weights to penalize misclassifying minority classes
    cw = class_weight.compute_class_weight(
        weight_strategy,
        classes=np.unique(Y_train.flatten()),
        y=Y_train.flatten()
    )
    class_weights = dict(enumerate(cw))

    # Create train and validation dataset
    train_ds = tf.data.Dataset.from_tensor_slices((X_train, Y_train))
    train_ds = train_ds.shuffle(10000).batch(batch_size).prefetch(tf.data.AUTOTUNE)

    val_ds = tf.data.Dataset.from_tensor_slices((X_val, Y_val))
    val_ds = val_ds.batch(batch_size).prefetch(tf.data.AUTOTUNE)

    return train_ds, val_ds, class_weights, scaler

def GPUorCPU(withgpu):
    # GPU vs CPU training
    if not withgpu:
        tf.config.set_visible_devices([], 'GPU')
        print(">>> Running on CPU only (GPU hidden)")
    else:
        gpus = tf.config.list_physical_devices('GPU')
        if gpus:
            print(">>> Running with GPU")
            try:
                for gpu in gpus:
                    tf.config.experimental.set_memory_growth(gpu, True)
            except RuntimeError as e:
                print(e)
            try:
                from tensorflow.keras import mixed_precision
                mixed_precision.set_global_policy('mixed_float16')
                print(">>> Mixed precision enabled")
            except:
                print(">>> Mixed precision not available")
        else:
            print(">>> No GPUs found!")

def dataset_to_numpy(dataset):
    X_list, Y_list = [], []
    for x_batch, y_batch in dataset:
        X_list.append(x_batch.numpy())
        Y_list.append(y_batch.numpy())
    X_np = np.concatenate(X_list, axis=0)
    Y_np = np.concatenate(Y_list, axis=0)
    return X_np, Y_np

# Main
if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description="Load data and labels from parquet files.")
    parser.add_argument('--data', type=str, required=True,
                        help='Path to the data parquet file')
    parser.add_argument('--labels', type=str, required=True,
                        help='Path to the labels parquet file')
    parser.add_argument('--num_steps', type=int, required=True,
                        help='Number of steps per sequence')
    parser.add_argument('--num_features', type=int,
                        required=True, help='Number of features per step')
    parser.add_argument('--batch_size', type=int, default=64,
                        help='Batch size (default: 64)')
    parser.add_argument('--withgpu', action='store_true',
                        help='Enable GPU usage')
    parser.add_argument('--class_weight_strategy', type=str, required=True,
                        help="Strategy for calculating class weights: balanced, None or a dictionary (es. '{0:1.0, 1:2.0}')")
    args = parser.parse_args()

    X_df = pd.read_parquet(args.data)
    Y_df = pd.read_parquet(args.labels)
    assert len(X_df) == len(Y_df)

    if args.class_weight_strategy == "None":
        weight_strategy = None
    elif args.class_weight_strategy == "balanced":
        weight_strategy = "balanced"
    elif args.class_weight_strategy.startswith("{"):
        import ast
        weight_strategy = ast.literal_eval(args.class_weight_strategy)
    else:
        raise ValueError("Not a valid value for --class_weight_strategy")

    GPUorCPU(args.withgpu)

    train_ds, val_ds, class_weights, scaler = spilt_scale_dataset(X_df, Y_df, args.num_steps, args.num_features, weight_strategy, args.batch_size)

    accperformance = train_model(train_ds, val_ds, args.num_steps, args.num_features,
                args.labels, args.batch_size, args.withgpu, class_weights, scaler, weight_strategy)
    print(accperformance)
