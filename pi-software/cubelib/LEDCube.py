import requests
import math
import random
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
    def __init__(self, host: str, width: int, brightness_bits: int = 4, port: int = 80):
        """A manager object for a remote rpi running a
        webserver.

        Args:
            host (str): The IP/Hostname of the Cube.
            width (int): The pixel width of the cube.
            port (int, optional): The port the pi is listening on. Defaults to 80.
        """
        self.host = host
        self.width = width
        self.brightness_bits = brightness_bits
        self.port = port

        # leds sequentially ordered
        self.matrix = [0] * self.width * self.width * self.width

        # Init animation counters
        self.clock_animation_xy = [0, 7]

    def get_coord(self, x: int, y: int, z: int) -> int:
        if x > self.width or y > self.width or z > self.width:
            raise ValueError("Coords out of range.")
        return self.matrix[(z * self.width * self.width) + (y * self.width) + x]

    def get_max_brightness(self) -> int:
        return int(math.pow(2, self.brightness_bits)) - 1

    def set_coord_brightness(self, x: int, y: int, z: int, brightness: int):
        if x >= self.width or y >= self.width or z >= self.width:
            raise ValueError("Coords out of range.")
        if brightness > math.pow(2, self.brightness_bits) - 1:
            brightness = self.get_max_brightness()
        self.matrix[(z * self.width * self.width) + (y * self.width) + x] = int(brightness)

    def set_coord(self, x: int, y: int, z: int, on: bool):
        brightness = 0
        if on:
            brightness = self.get_max_brightness()
        self.set_coord_brightness(x, y, z, brightness)

    def get_hex_matrix(self) -> str:
        result = ""
        for z in range(self.width):
            for y in range(self.width):
                for br_bit in range(self.brightness_bits):
                    char = 0
                    for x in range(self.width):
                        br_value = self.get_coord(x, y, z)
                        # i know this is super weird, but hear me out:
                        # this is applying a bit mask to the numerical value of brightness
                        # based on the current brightness "phase" which we are calculating.
                        # So we are adding the value for every bit in the binary
                        # representation of our number. The cube then illuminates the pixel
                        # for every brightness frame so they add up to an illumination time
                        # that corresponds to the numerical value. Still dont get it? :(
                        if (br_value & int(math.pow(2, br_bit))):
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
            str(int(now.hour/10))), 0, 1, 0, PlaneOrientation.YZP)
        self.set_plane(self.two_by_seven_char(
            str(int(now.hour % 10))), 0, 5, 0, PlaneOrientation.YZP)
        # two digit minutes
        self.set_plane(self.two_by_seven_char(
            str(int(now.minute/10))), 7, 0, 0, PlaneOrientation.XZN)
        self.set_plane(self.two_by_seven_char(
            str(int(now.minute% 10))), 3, 0, 0, PlaneOrientation.XZN)
        # two digit seconds
        self.set_plane(self.two_by_seven_char(
            str(int(now.second/10))), 7, 7, 0, PlaneOrientation.YZN)
        self.set_plane(self.two_by_seven_char(
            str(int(now.second% 10))), 7, 3, 0, PlaneOrientation.YZN)

        if now.second == 0 and self.clock_animation_xy[0] == 0 and self.clock_animation_xy[0] == 0:
            # circle animation
            pass

    def get_url(self) -> str:
        return f"http://{self.host}:{self.port}/display"

    def update(self):
        response = requests.post(self.get_url(), data=self.get_hex_matrix())

        if response.status_code != 200:
            print(str(response))
            raise RuntimeError("The webserver gave non ok response.")

    def set_random(self):
        for x in range(self.width):
            for y in range(self.width):
                for z in range(self.width):
                    self.set_coord_brightness(x, y, z, random.randint(0, self.get_max_brightness()))

    def set_full_cube(self, brightness: int):
        for x in range(self.width):
            for y in range(self.width):
                for z in range(self.width):
                    self.set_coord_brightness(x, y, z, brightness)


if __name__ == "__main__":
    from time import sleep
    cube = LEDCube("ledcube", 8)
    cube.off()
    while True:
        cube.clock_animation()
        cube.update()
        sleep(0.1)
