#include <stdio.h>
#include <stdlib.h>
#include <libc.h>
#include <stdbool.h>
#include "HashMap.h"
#include "HashCode.h"
#include "Equal.h"

#include <jpeglib.h>

struct ImageData {
    unsigned char *pixels;
    long width;
    long height;
};

//链接：https://juejin.cn/post/6844904099620585479

int decode_JPEG_file(char *inJpegName, char *outRgbName) {
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    FILE *infile;
    FILE *outfile;

    if ((infile = fopen(inJpegName, "rb")) == NULL) {
        fprintf(stderr, "can't open %s\n", inJpegName);
        return -1;
    }
    if ((outfile = fopen(outRgbName, "wb")) == NULL) {
        fprintf(stderr, "can't open %s\n", outRgbName);
        return -1;
    }

    cinfo.err = jpeg_std_error(&jerr);

    jpeg_create_decompress(&cinfo);

    jpeg_stdio_src(&cinfo, infile);

    jpeg_read_header(&cinfo, TRUE);

    printf("image_width = %d\n", cinfo.image_width);
    printf("image_height = %d\n", cinfo.image_height);
    printf("num_components = %d\n", cinfo.num_components);
    printf("enter scale M/N:\n");

    jpeg_start_decompress(&cinfo);

    //输出的图象的信息
    printf("output_width = %d\n", cinfo.output_width);
    printf("output_height = %d\n", cinfo.output_height);
    printf("output_components = %d\n", cinfo.output_components);

    int row_stride = cinfo.output_width * cinfo.output_components;
    /* Make a one-row-high sample array that will go away when done with image */
    JSAMPARRAY buffer = (JSAMPARRAY) malloc(sizeof(JSAMPROW));
    buffer[0] = (JSAMPROW) malloc(sizeof(JSAMPLE) * row_stride);

    struct ImageData imageData = {
            .width =  cinfo.image_width,
            .height = cinfo.image_height,
            .pixels = malloc(row_stride * cinfo.image_height)
    };
    long counter = 0;

    while (cinfo.output_scanline < cinfo.output_height) {
        jpeg_read_scanlines(&cinfo, buffer, 1);
        memcpy(imageData.pixels + counter, buffer[0], row_stride);
        counter += row_stride;
    }

    printf("total size: %ld\n", counter);
    fwrite(imageData.pixels, counter, 1, outfile);


    jpeg_finish_decompress(&cinfo);

    jpeg_destroy_decompress(&cinfo);

    fclose(infile);
    fclose(outfile);
    free(imageData.pixels);

    return 0;
}


unsigned char clip_value(unsigned char x, unsigned char min_val, unsigned char max_val) {
    if (x > max_val) {
        return max_val;
    } else if (x < min_val) {
        return min_val;
    } else {
        return x;
    }
}

//RGB to YUV420
bool RGB24_TO_YUV420(unsigned char *RgbBuf, int w, int h, unsigned char *yuvBuf) {
    unsigned char *ptrY, *ptrU, *ptrV, *ptrRGB;
    memset(yuvBuf, 0, w * h * 3 / 2);
    ptrY = yuvBuf;
    ptrU = yuvBuf + w * h;
    ptrV = ptrU + (w * h * 1 / 4);
    unsigned char y, u, v, r, g, b;
    for (int j = 0; j < h; j++) {
        ptrRGB = RgbBuf + w * j * 3;
        for (int i = 0; i < w; i++) {

            r = *(ptrRGB++);
            g = *(ptrRGB++);
            b = *(ptrRGB++);
            y = (unsigned char) ((66 * r + 129 * g + 25 * b + 128) >> 8) + 16;
            u = (unsigned char) ((-38 * r - 74 * g + 112 * b + 128) >> 8) + 128;
            v = (unsigned char) ((112 * r - 94 * g - 18 * b + 128) >> 8) + 128;
            *(ptrY++) = clip_value(y, 0, 255);
            if (j % 2 == 0 && i % 2 == 0) {
                *(ptrU++) = clip_value(u, 0, 255);
            } else {
                if (i % 2 == 0) {
                    *(ptrV++) = clip_value(v, 0, 255);
                }
            }
        }
    }
    return true;
}

/**
 * Convert RGB24 file to YUV420P file
 * @param url_in  Location of Input RGB file.
 * @param w       Width of Input RGB file.
 * @param h       Height of Input RGB file.
 * @param num     Number of frames to process.
 * @param url_out Location of Output YUV file.
 */
int simplest_rgb24_to_yuv420(char *url_in, int w, int h, int num, char *url_out) {
    FILE *fp = fopen(url_in, "rb+");
    FILE *fp1 = fopen(url_out, "wb+");

    unsigned char *pic_rgb24 = (unsigned char *) malloc(w * h * 3);
    unsigned char *pic_yuv420 = (unsigned char *) malloc(w * h * 3 / 2);

    for (int i = 0; i < num; i++) {
        fread(pic_rgb24, 1, w * h * 3, fp);
        RGB24_TO_YUV420(pic_rgb24, w, h, pic_yuv420);
        fwrite(pic_yuv420, 1, w * h * 3 / 2, fp1);
    }

    free(pic_rgb24);
    free(pic_yuv420);
    fclose(fp);
    fclose(fp1);

    return 0;
}

/// 转16进制
void convert(unsigned char quotient, unsigned char *hexadecimalNumber) {

    unsigned char i = 0, temp;

    hexadecimalNumber[0] = 48;
    hexadecimalNumber[1] = 48;

    while (quotient != 0) {
        temp = quotient % 16;

        if (temp < 10)
            temp = temp + 48;
        else
            temp = temp + 55;

        *(hexadecimalNumber++) = temp;

        quotient = quotient / 16;
    }
}

/// 16进制颜色表示
void contact(unsigned char r, unsigned char g, unsigned char b, unsigned char *hex) {

    unsigned char *rs = (unsigned char *) malloc(2);
    convert(r, rs);
    unsigned char *gs = (unsigned char *) malloc(2);
    convert(g, gs);
    unsigned char *bs = (unsigned char *) malloc(2);
    convert(b, bs);

    *(hex) = *(rs + 1);
    *(hex + 1) = *(rs);
    *(hex + 2) = *(gs + 1);
    *(hex + 3) = *(gs);
    *(hex + 4) = *(bs + 1);
    *(hex + 5) = *(bs);
}

void pt(Entry *entry) {
    printf("%s %d \n", (char *) entry->key, entry->value);
}

void bubble_sort(Entry **arr, int len) {
    int i, j;
    Entry *temp;
    for (i = 0; i < len - 1; i++)
        for (j = 0; j < len - 1 - i; j++)
            if (arr[j]->value < arr[j + 1]->value) {
                temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
}

/// top 颜色
int topXX(char *url_in, int w, int h, int num) {
    FILE *fp = fopen(url_in, "rb+");
    unsigned char *pic_rgb24 = (unsigned char *) malloc(w * h * 3);

    fread(pic_rgb24, 1, w * h * 3, fp);

    unsigned char *ptrRGB;

    MyHashMap *myHashmap = createMyHashMap(myHashCodeString, myEqualString);

//    unsigned char *hex = (unsigned char *) malloc(6);

    unsigned char r, g, b;
    for (int j = 0; j < h; j++) {
        ptrRGB = pic_rgb24 + w * j * 3;
        for (int i = 0; i < w; i++) {
            r = *(ptrRGB++);
            g = *(ptrRGB++);
            b = *(ptrRGB++);

            unsigned char *hex = (unsigned char *) malloc(6);

            contact(r, g, b, hex);
//            printf("--%s\n", hex);

            int count = myHashMapGetDataByKey(myHashmap, hex);
            if (count != NULL) {
                int newCount = count + 1;
                myHashMapPutData(myHashmap, hex, newCount);
            } else {
                int newCount = 1;
                myHashMapPutData(myHashmap, hex, newCount);
            }

//            free(hex);
        }
    }

    myHashMapGetSize(myHashmap);

//    myHashMapOutput(myHashmap, pt);
//    printf("--%d",myHashmap->size);
    Entry **list = (Entry **) malloc(sizeof(Entry *) * myHashmap->size);

    MyHashMapEntryIterator *iterator = createMyHashMapEntryIterator(myHashmap);
    while (myHashMapEntryIteratorHasNext(iterator)) {
        Entry *entry = myHashMapEntryIteratorNext(iterator);
//        pt(entry);
//        printf("--%d",iterator->count);

        list[iterator->count-1] = entry;
    }

    bubble_sort(list, myHashmap->size);

    for (int i = 0; i < num; ++i) {
        pt(list[i]);
    }

    freeMyHashMapEntryIterator(iterator);

//    free(hex);

    free(list);

    free(pic_rgb24);
    fclose(fp);

    return 0;
}

int main() {
//    printf("Hello, World!\n");

    /// libjpeg-turbo jpeg -> rgb24
    char *inJpegName1 = "/Users/blackox626/CLionProjects/RGBAndYUV/resource/followw813654-00290000017f4f7cbcf50a2198f4_512_512.jpeg";
    char *outRgbName1 = "/Users/blackox626/CLionProjects/RGBAndYUV/resource/libjpeg-turbo-test-image.rgb24";
    int flag1 = decode_JPEG_file(inJpegName1, outRgbName1);
    if (flag1 == 0) {
        printf("decode ok!\n");
    } else {
        printf("decode error!\n");
    }
    printf("↑↑↑↑↑↑↑↑↑↑ Decode JPEG to RGB24 ↑↑↑↑↑↑↑↑↑↑\n\n");

    /// ffplay -f rawvideo -pixel_format rgb24  -s 3024x4032 /Users/blackox626/CLionProjects/RGBAndYUV/resource/libjpeg-turbo-test-image.rgb24

    char *outYUVName1 = "/Users/blackox626/CLionProjects/RGBAndYUV/resource/libjpeg-turbo-test-image.yuv";

    simplest_rgb24_to_yuv420(outRgbName1, 3024, 4032, 1, outYUVName1);

    /// YUVEye 也可以查看
    /// ffplay -f rawvideo -pixel_format yuv420p  -s 3024x4032 /Users/blackox626/CLionProjects/RGBAndYUV/resource/libjpeg-turbo-test-image.yuv

    /// 可以看到 YUV 是 RGB的一半大小
    /// RGB size = width * heihht * 3 (byte)
    /// YUV size = width * heihht * 1.5 (byte)  420p ( y 1, uv 0.5)

    /// top 20 color
    topXX(outRgbName1, 1200, 800, 20);

    return 0;
}
