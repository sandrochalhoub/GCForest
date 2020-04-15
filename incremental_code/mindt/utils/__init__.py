from .constants import *

import os

def create_directory(dir):
    try:
        os.makedirs(dir)
    except FileExistsError:
        pass
    return dir
