import ulab.numpy as np

class Kalman2D:
    def __init__(self):
        # 状态 [x, y, vx, vy]
        self.x = np.zeros((4,1))

        self.P = np.eye(4) * 100.0

        self.A = np.array([
            [1,0,1,0],
            [0,1,0,1],
            [0,0,1,0],
            [0,0,0,1]
        ])

        self.H = np.array([
            [1,0,0,0],
            [0,1,0,0]
        ])

        self.Q = np.diag([0.01, 0.01, 0.1, 0.1])
        self.R = np.diag([4.0, 4.0])

        self.I = np.eye(4)
        self.inited = False

    def update(self, zx, zy):
        z = np.array([[zx],[zy]])

        if not self.inited:
            self.x[0,0] = zx
            self.x[1,0] = zy
            self.inited = True
            return zx, zy

        # predict
        self.x = self.A @ self.x
        self.P = self.A @ self.P @ self.A.T + self.Q

        # update
        y = z - self.H @ self.x
        S = self.H @ self.P @ self.H.T + self.R
        K = self.P @ self.H.T @ np.linalg.inv(S)

        self.x = self.x + K @ y
        self.P = (self.I - K @ self.H) @ self.P

        return int(self.x[0,0]), int(self.x[1,0])
