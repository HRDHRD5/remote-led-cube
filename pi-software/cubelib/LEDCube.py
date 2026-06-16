import requests
import math
from datetime import datetime
from enum import Enum


TWO_BY_SEVEN_MAP = {
    "0": [[False, False, True, True, True, True, True], [False, False, True, False, False, False, True], [False, False, True, True, True, True, True]],
    "1": [[False, False, False, False, False, True, False], [False, False, True, True, True, True, True], [False, False, False, False, False, False, False]],
    "2": [[False, False, True, True, True, False, True], [False, False, True, False, True, False, True], [False, False, True, False, True, True, True]],
    "3": [[False, False, True, False, True, False, True], [False, False, True, False, True, False, True], [False, False, True, True, True, True, True]],
    "4": [[False, False, False, False, True, True, True], [False, False, False, False, True, False, False], [False, False, True, True, True, True, True]],
    "5": [[False, False, True, False, True, True, True], [False, False, True, False, True, False, True], [False, False, False, True, True, False, True]],
    "6": [[False, False, True, True, True, True, True], [False, False, True, False, True, False, True], [False, False, True, True, True, False, True]],
    "7": [[False, False, False, False, False, False, True], [False, False, False, False, False, False, True], [False, False, True, True, True, True, True]],
    "8": [[False, False, True, True, True, True, True], [False, False, True, False, True, False, True], [False, False, True, True, True, True, True]],
    "9": [[False, False, False, False, True, True, True], [False, False, False, False, True, False, True], [False, False, True, True, True, True, True]],
}


# maximum Packet sending Rate in Hz
RECOMMENDED_FREQUENCY = 10


class PlaneOrientation(Enum):
    XYP = 1
    XZP = 2
    YZP = 3
    XYN = 4
    XZN = 5
    YZN = 6
    XYI = 7
    XZI = 8
    YZI = 9


class LEDCube:
    def __init__(self, host: str, width: int, port: int = 80):
        """A manager object for a remote rpi running a
        webserver.

        Args:
            host (str): The IP/Hostname of the Cube.
            width (int): The pixel width of the cube.
            port (int, optional): The port the pi is listening on. Defaults to 80.
        """
        self.host = host
        self.width = width
        self.port = port

        # leds sequentially ordered
        self.matrix = [True] * self.width * self.width * self.width

        # Init animation counters
        self.clock_animation_xy = [0, 7]

    def get_coord(self, x: int, y: int, z: int) -> bool:
        if x > self.width or y > self.width or z > self.width:
            raise ValueError("Coords out of range.")
        return self.matrix[(z * self.width * self.width) + (y * self.width) + x]

    def set_coord(self, x: int, y: int, z: int, on: bool):
        if x > self.width or y > self.width or z > self.width:
            raise ValueError("Coords out of range.")
        self.matrix[(z * self.width * self.width) + (y * self.width) + x] = on

    def get_hex_matrix(self) -> str:
        result = ""
        for z in range(self.width):
            for y in range(self.width):
                char = 0
                for x in range(self.width):
                    if (self.get_coord(x, y, z)):
                        char += int(math.pow(2, x))
                result += "{:02X}".format(char)
        return result

    def set_plane(self, plane: list[list[bool]], x: int, y: int, z: int, orientation: PlaneOrientation):
        for a, val_a in enumerate(plane):
            if orientation not in [PlaneOrientation.XYP, PlaneOrientation.XZP, PlaneOrientation.YZP]:
                a = -a
            for b, val_pixel in enumerate(val_a):
                if orientation in [PlaneOrientation.XYI, PlaneOrientation.XZI, PlaneOrientation.YZI]:
                    b = -b

                if orientation in [PlaneOrientation.XYP, PlaneOrientation.XYN, PlaneOrientation.XYI]:
                    self.set_coord(x+a, y+b, z, val_pixel)
                elif orientation in [PlaneOrientation.XZP, PlaneOrientation.XZN, PlaneOrientation.XZI]:
                    self.set_coord(x+a, y, z+b, val_pixel)
                else:
                    self.set_coord(x, y+a, z+b, val_pixel)

    def set_line(self, x0: int, y0: int, z0: int, x1: int, y1: int, z1: int, on: bool=True):
        # implement 3D Bresenham T_T
        pass

    def on(self):
        for x in range(self.width):
            for y in range(self.width):
                for z in range(self.width):
                    self.set_coord(x, y, z, True)

    def off(self):
        for x in range(self.width):
            for y in range(self.width):
                for z in range(self.width):
                    self.set_coord(x, y, z, False)

    def two_by_seven_char(self, char: str):
        if char not in TWO_BY_SEVEN_MAP:
            raise ValueError("Non supported character")
        return TWO_BY_SEVEN_MAP[char]

    def clock_animation(self):
        cube.off()
        now = datetime.now()

        # two digit hours
        self.set_plane(self.two_by_seven_char(
            str(int(now.hour/10))), 0, 7, 0, PlaneOrientation.YZN)
        self.set_plane(self.two_by_seven_char(
            str(int(now.hour % 10))), 0, 4, 0, PlaneOrientation.YZN)
        # two digit minutes
        self.set_plane(self.two_by_seven_char(
            str(int(now.minute/10))), 0, 0, 0, PlaneOrientation.XZP)
        self.set_plane(self.two_by_seven_char(
            str(int(now.minute% 10))), 4, 0, 0, PlaneOrientation.XZP)
        # two digit seconds
        self.set_plane(self.two_by_seven_char(
            str(int(now.second/10))), 7, 1, 0, PlaneOrientation.YZP)
        self.set_plane(self.two_by_seven_char(
            str(int(now.second% 10))), 7, 5, 0, PlaneOrientation.YZP)

        if now.second == 0 and self.clock_animation_xy[0] == 0 and self.clock_animation_xy[0] == 0:
            # circle animation
            pass

    def get_url(self) -> str:
        return f"http://{self.host}:{self.port}/"

    def update(self):
        response = requests.post(self.get_url(), data=self.get_hex_matrix())

        if response.status_code != 200:
            raise RuntimeError("The webserver gave non ok response.")


if __name__ == "__main__":
    from time import sleep
    cube = LEDCube("10.42.0.211", 8)
    cube.off()
    while True:
        cube.clock_animation()
        cube.update()
        sleep(0.1)
