from rules import *
import numpy as np


class Rulemds:
    def __init__(self, ARTenabled):
        self.ARTenabled = ARTenabled

    def extrapolatePos(self, cam, deltaT):
        accX = cam.acl * np.cos(np.radians(cam.hed))
        accY = cam.acl * np.sin(np.radians(cam.hed))
        posX_est = cam.posx + cam.spdx * deltaT + 0.5 * accX * (deltaT ** 2)
        posY_est = cam.posy + cam.spdy * deltaT + 0.5 * accY * (deltaT ** 2)
        return posX_est, posY_est


    def retrievePos(self, time, gt):
        lastvehcam = gt[gt.sendTime < time].iloc[-1]
        ageofcam = time - lastvehcam.sendTime
        assert ageofcam > 0 and ageofcam <= 1.1
        x, y = self.extrapolatePos(lastvehcam, ageofcam)
        return x, y

    def evaluate_message_pair(self, msg_prev, msg, mem_score, rxveh_gt):
        msg_prev = Message(*msg_prev)
        msg = Message(*msg)
        posx, posy = self.retrievePos(time=msg.sendTime, gt=rxveh_gt)

        metrics, mem_score, scores, errors = evaluate_message_pair(
                msg_prev, msg, mem_score, (posx, posy), self.ARTenabled)

        return metrics, mem_score, scores, errors

    def predict(self, metrics, scores):
        pred = 1 if metrics['norm_sum'] >= 1/len(scores) else 0
        weight = self.computeWeight(metrics['norm_sum'], len(scores))
        return (pred, weight)

    def computeWeight(self, score, n):
        min_val = 1 / n
        left_knee = 1 / (2 * n)
        right_knee = (1/n) + (1/2) * (1 - 1/n)

        if score <= left_knee:
            return 1
        elif score <= min_val:
            # Linear from 1 to min_val
            return 1 - (score - left_knee) / (min_val - left_knee) * (1 - min_val)
        elif score <= right_knee:
            # Linear from min_val to 1
            return min_val + (score - min_val) / (right_knee - min_val) * (1 - min_val)
        else:
            return 1
