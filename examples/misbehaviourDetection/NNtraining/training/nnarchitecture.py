from keras.models import Sequential
from keras.layers import Dense, Dropout, BatchNormalization, Input, LSTM, Conv1D
from keras.optimizers import Adam, AdamW
from keras.callbacks import EarlyStopping, ReduceLROnPlateau

# Callbacks
es = EarlyStopping(
    monitor='val_accuracy',
    patience=10,
    restore_best_weights=True,
    mode='max',
    verbose=1
)

lr = ReduceLROnPlateau(
    monitor='val_accuracy',
    factor=0.5,
    patience=5,
    min_lr=1e-6,
    mode='max',
    verbose=1
)

def build_mlp(num_steps=4, num_features=6):
    model = Sequential([
        Input(shape=(num_steps, num_features)),
        Conv1D(64, kernel_size=3, activation='relu'),
        BatchNormalization(),
        LSTM(64, return_sequences=False),
        Dropout(0.1),
        Dense(64, activation="relu"),
        Dropout(0.1),
        Dense(9, activation="softmax")
    ])

    model.compile(
        optimizer=Adam(learning_rate=0.001),
        loss='sparse_categorical_crossentropy',
        metrics=['accuracy']
    )

    return model


def build_old_mlp(num_steps=4, num_features=6):
    model = Sequential([
      Input(shape=(num_steps, num_features)),
      LSTM(256, return_sequences=False),
      Dense(156, activation="relu"),
      Dense(9, activation="softmax")
    ])

    model.compile(AdamW(learning_rate=0.001),
                  loss='sparse_categorical_crossentropy',
                  metrics=['accuracy']
                )
    return model
