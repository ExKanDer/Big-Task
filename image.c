#include "lodepng.c"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct point{
    int x, y;
} Point;

char* loadPNGFile(const char *filename, int *width, int *height) {
	unsigned char *image = NULL;
	int error = lodepng_decode32_file(&image, width, height, filename);
	if (error) {
		printf("error %u: %s\n", error, lodepng_error_text(error));
		return NULL;
	}
	return (char*)image;
}

void writePNGFile(const char *filename, const unsigned char *image, unsigned width, unsigned height) {
    unsigned char* png;
    size_t pngSize;
    int error = lodepng_encode32(&png, &pngSize, image, width, height);
    if (!error) {
        lodepng_save_file(png, pngSize, filename);
    }
    free(png);
}

void applyRandomColour(unsigned char* image, int width, int height, int threshold) {
    for(int row = 1; row < height - 1; row++) {
        for(int col = 1; col < width - 1; col++) {
            int colourRed = rand() % (255 - threshold * 2) + threshold * 2;
            int colourGreen = rand() % (255 - threshold * 2) + threshold * 2;
            int colourBlue = rand() % (255 - threshold * 2) + threshold * 2;
            if(image[4 * (row * width + col)] < threshold || image[4 * (row * width + col)] > 255 - (threshold * 2)) {
                int deltaX[] = {-1, 0, 1, 0};
                int deltaY[] = {0, 1, 0, -1};
                Point* stack = malloc(width * height * 4 * sizeof(Point));
                long stackTop = 0;
                stack[stackTop++] = (Point){col, row};
                while(stackTop > 0) {
                    Point currentPixel = stack[--stackTop];
                    if(currentPixel.x < 0 || currentPixel.x >= width || currentPixel.y < 0 || currentPixel.y >= height) continue;
                    int index = (currentPixel.y * width + currentPixel.x) * 4;
                    if(image[index] > threshold) continue;
                    image[index] = colourRed;
                    image[index + 1] = colourGreen;
                    image[index + 2] = colourBlue;
                    for(int direction = 0; direction < 4; direction++) {
                        int newX = currentPixel.x + deltaX[direction];
                        int newY = currentPixel.y + deltaY[direction];
                        if(newX > 0 && newX < width && newY > 0 && newY < height) {
                            stack[stackTop++] = (Point){newX, newY};
                        }
                    }
                }
                free(stack);
            }
        }
    }
}

void applySobelFilter(unsigned char *image, int width, int height) {
    int kernelX[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
    int kernelY[3][3] = {{1, 2, 1}, {0, 0, 0}, {-1, -2, -1}};
    unsigned char *tempImage = malloc(width * height * 4 * sizeof(unsigned char));
    for(int y = 1; y < height - 1; y++) {
        for(int x = 1; x < width - 1; x++) {
            int gradientX = 0;
            int gradientY = 0;
            for(int i = -1; i <= 1; i++) {
                for(int j = -1; j <= 1; j++) {
                    int index = ((y + j) * width + (i + x)) * 4;
                    int rgbAverage = (image[index] + image[index + 1] + image[index + 2]) / 3;
                    gradientX += kernelX[j + 1][i + 1] * rgbAverage;
                    gradientY += kernelY[j + 1][i + 1] * rgbAverage;
                }
            }
            int gradientMagnitude = sqrt(gradientX * gradientX + gradientY * gradientY);
            if(gradientMagnitude > 255) {
                gradientMagnitude = 255;
            }
            int resultIndex = (y * width + x) * 4;
            tempImage[resultIndex] = (unsigned char)gradientMagnitude;
            tempImage[resultIndex + 1] = (unsigned char)gradientMagnitude;
            tempImage[resultIndex + 2] = (unsigned char)gradientMagnitude;
            tempImage[resultIndex + 3] = image[resultIndex + 3];
        }
    }
    for(int i = 0; i < width * height * 4; i++) {
        image[i] = tempImage[i];
    }
    free(tempImage);
}

unsigned char *convertToGrayscale(unsigned char* image, unsigned width, unsigned height) {
    unsigned char maxRed = 0, maxGreen = 0, maxBlue = 0;
    unsigned char red, green, blue, alpha;
    for(int i = 0; i < 4 * height * width; i += 4) {
        red = image[i];
        green = image[i + 1];
        blue = image[i + 2];
        alpha = image[i + 3];
        maxRed = fmax(red, maxRed);
        maxGreen = fmax(green, maxGreen);
        maxBlue = fmax(blue, maxBlue);
    }
    for(int i = 0; i < 4 * width * height; i += 4) {
        red = image[i];
        green = image[i + 1];
        blue = image[i + 2];
        alpha = image[i + 3];
        unsigned char grayscaleValue = 255.0 * ((float)red / maxRed + (float)green / maxGreen + (float)blue / maxBlue) / 3;
        image[i] = image[i + 1] = image[i + 2] = grayscaleValue;
    }
    return image;
}

int main() {
    int width = 0, height = 0;
    double threshold = 25.0;
    const char *inputFilename = "skull.png";
    const char *outputFilename = "skull-result.png";
    unsigned char *originalImage = loadPNGFile(inputFilename, &width, &height);
    unsigned char *grayscaleImage = convertToGrayscale(originalImage, width, height);
    applySobelFilter(grayscaleImage, width, height);
    applyRandomColour(grayscaleImage, width, height, threshold);
    writePNGFile(outputFilename, grayscaleImage, width, height);
    free(originalImage);
    free(grayscaleImage);
    return 0;
}
