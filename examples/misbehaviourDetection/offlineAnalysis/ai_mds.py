from tensorflow.keras.models import load_model
import pickle
import numpy as np
from scipy.stats import t
import tensorflow as tf
import code  # code.interact(local=dict(globals(), **locals()))

def enable_mc_dropout(model):
    for layer in model.layers:
        if isinstance(layer, tf.keras.layers.Dropout):
            layer.trainable = True
        else:
            layer.trainable = False
    return model

class Aimds:
    def __init__(self, model_path, scaler_path, MCdropRep, MCdropCU):
        model = load_model(model_path)
        model = enable_mc_dropout(model)
        self.model = model

        with open(scaler_path, "rb") as f:
            scaler = pickle.load(f)
        self.scaler = scaler

        self.MCdropRep = MCdropRep
        self.MCdropCU = MCdropCU


    def evaluate_sequence(self, x, y):
        freedom_degrees = self.MCdropRep - 2
        uncertainty_rel = self.MCdropCU

        true_labels = []
        predicted_labels = []
        pointestimates = []
        confidence_levels = []

        X_scaled = self.scaler.transform(x.reshape(-1,24)).reshape(-1, 4, 6)

        preds = []
        for i in range(self.MCdropRep):
            probs = self.model(X_scaled, training=True).numpy().squeeze()
            preds.append(probs)

        preds = np.stack(preds)
        mean_probs = preds.mean(axis=0)
        std_probs = preds.std(axis=0)

        predicted_label = int(np.argmax(mean_probs))
        mu = mean_probs[predicted_label]
        sigma = std_probs[predicted_label]

        if sigma == 0:
            CL = 1.0  # or 0.999999, depending on how cautious you are
        else:
            # Find the t-critic that produces an interval of desired relative width
            t_observed = (mu * uncertainty_rel) * np.sqrt(self.MCdropRep) / sigma
            p = t.cdf(t_observed, df=freedom_degrees)
            CL = 2 * p - 1 # Confidence level that generates that interval

        return predicted_label, mu, CL