#include "lodepng.c"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct point{
    int x, y;
}pt;

char* load_png_file(const char *filename, int *width, int *height) {
	unsigned char *image = NULL;
	int error = lodepng_decode32_file(&image, width, height, filename);
	if (error) {
		printf("error %u: %s\n", error, lodepng_error_text(error));
		return NULL;
	}
	return (image);
}

void write_png_file(const char *filename, const unsigned char *image, unsigned width, unsigned height) {
    unsigned char* png;
    size_t size;
    int error = lodepng_encode32(&png, &size, image, width, height);
    if (!error) {
        lodepng_save_file(png, size, filename);
    }
    free(png);
}

void colouring(unsigned char* image, int width, int height, int eps)
{

    pt* stack = malloc(width * height * 4 * sizeof(pt));
    long stack_size = 0;

    if (stack == NULL) {
        printf("Not enough memory for 'colouring(image,width=%d,height=%d,eps) function!\n", width, height);
        return;
    }

    int color_comps[3] = { 0 };
    int matrix[2][4] = { {-1,0,1,0},{0,1,0,-1} };

    for (int j = 1; j < height - 1; j++) {
        for (int i = 1; i < width - 1; i++) {
            for (int k = 0; k < 3; ++k) {
                color_comps[k] = rand() % (250 - eps * 2) + eps * 2;
            }
            if (image[4 * (j * width + i)] < eps || image[4 * (j * width + i)] > 255 - (eps * 2)) {

                stack[stack_size++] = (pt){ i, j };

                while (stack_size > 0) {

                    pt pixel = stack[--stack_size];

                    if (pixel.x < 0 || pixel.x >= width || pixel.y < 0 || pixel.y >= height) continue;
                    int ind = (pixel.y * width + pixel.x) * 4;
                    if (image[ind] > eps) continue;

                    for (int k = 0; k < 3; ++k) {
                        image[ind + k] = color_comps[k];
                    }

                    for (int i = 0; i < 4; i++) {

                        pt new_pixel = (pt){ pixel.x + matrix[0][i],  pixel.y + matrix[1][i] };

                        if (new_pixel.x > 0 && new_pixel.x < width && new_pixel.y > 0 && new_pixel.y < height) {
                            stack[stack_size++] = new_pixel;
                        }
                    }
                }
            }
        }
    }

    free(stack);
}




void Filter(unsigned char* image, int width, int height) {


    int kernel_matrix[2][3][3] = { { {-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1} }, { {1, 2, 1}, {0, 0, 0}, {-1, -2, -1} } };


    unsigned char* temp_image = malloc(width * height * 4 * sizeof(unsigned char));
    if (temp_image == NULL) {
        printf("Not enough memory for Filter(image,width=%d,height=%d)\n", width, height);
        return;
    }

    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {

            int vec[2] = { 0, 0 };

            for (int i = -1; i <= 1; i++) {
                for (int j = -1; j <= 1; j++) {

                    int ind = ((y + j) * width + (i + x)) * 4;
                    int rgb = (int)round((image[ind] + image[ind + 1] + image[ind + 2]) / 3.0);

                    for (int k = 0; k < 2; ++k) {
                        vec[k] += kernel_matrix[k][j + 1][i + 1] * rgb;
                    }
                }
            }

            int grad = (int)round(sqrt(vec[0]*vec[0] + vec[1]*vec[1]));

            if (grad > 255) {
                grad = 255;
            }

            int byte_offset = (y * width + x) * 4;

            for (int k = 0; k < 3; ++k) {
                temp_image[byte_offset+k] = (unsigned char)grad;
            }
            temp_image[byte_offset + 3] = image[byte_offset + 3];
        }
    }
    for (int k = 0; k < width * height * 4; k++) {
        image[k] = temp_image[k];
    }
    free(temp_image);
    return;
}


int main()
{
	unsigned w = 0, h = 0;
	int k = 0;
	double epsilon = 25.0;
	unsigned char *filename = "skull.png";
	unsigned char *output = "skull-result.png";
	unsigned char *image = load_png_file(filename, &w, &h);
	Filter(image, w, h);
    colouring(image,w,h,epsilon);
	write_png_file(output, image, w, h);
	free(image);
	return 0;
}
