import cv2
import numpy as np
import os

CURRENT_FILE_PATH = os.path.realpath(__file__)
EXTRAS_PATH = os.path.dirname(CURRENT_FILE_PATH)
REPO_ROOT = os.path.dirname(EXTRAS_PATH)
BENCHMARK_BASE_PATH = 'benchmark-dataset'
BENCHMARK_BASE_PATH = os.path.join(REPO_ROOT, BENCHMARK_BASE_PATH)

FILE_TO_GET_FROM = 'Dragonfly-1-masked.jpg'


def main():
    file_path = os.path.join(BENCHMARK_BASE_PATH, FILE_TO_GET_FROM)
    mask = get_mask(file_path)

    cv2.imshow('m', mask)
    cv2.waitKey()


def get_mask(file_path, red_color_min=224, other_color_max=31, mask_val=255):
    img = cv2.imread(file_path)
    mask = np.zeros((img.shape[0], img.shape[1]), dtype=np.uint8)

    for i in range(img.shape[0]):
        for j in range(img.shape[1]):
            if (img[i, j, 2] >= red_color_min) and (img[i, j, 0] <= other_color_max) and (img[i, j, 1] <= other_color_max):
                mask[i, j] = mask_val

    return mask

if __name__ == '__main__':
    main()
