/*
 * v4l2.c
 *
 *  Created on: Aug 8, 2013
 *      Author: liaoxinpeng
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <linux/videodev2.h>
//#include <jpeglib.h>
#include <linux/fb.h>

#include "camerainit.h"

static int fd = -1;

struct buffer {
        void *                  start;
        size_t                  length;
};

//static io_method        io              = IO_METHOD_MMAP;

struct buffer *         buffers1         = NULL;
static unsigned int     n_buffers       = 0;

// global settings

static unsigned int width = 800;
static unsigned int height = 480;
static char* deviceName = "/dev/video0";
char image_buf[1536000+54];

unsigned char bmp_h[] ={
                0x42,0x4d, //BM
                0x42,0x70,0x17,0x00, // 172000+66
                0x00,0x00,0x00,0x00,
                0x36,0x00,0x00,0x00,  //bmp_data offset
                0x28,0x00,0x00,0x00,
                0x20,0x03,0x00,0x00,   //width
                0xE0,0x01,0x00,0x00,   //hieght
                0x01,0x00,
                0x20,0x00,   //32bit
                0x00,0x00,0x00,0x00,
                0x00,0x70,0x17,0x00,  //bmp_data size
                0x00,0x00,0x00,0x00,
                0x00,0x00,0x00,0x00,
                0x00,0x00,0x00,0x00,
                0x00,0x00,0x00,0x00,
};

void deviceOpen(void)
{
    fd = open(deviceName, O_RDWR /* required */ | O_NONBLOCK, 0);

    // check if opening was successfull
    if (-1 == fd)
    {
          fprintf(stderr, "Cannot open '%s': %d, %s\n", deviceName, errno, strerror (errno));
          qDebug() << "FAILED open camera";
    }
    else
    {
        qDebug()<<"success open camera";
    }
}

/**
  initialize device
*/
void deviceInit(void)
{
    struct v4l2_capability cap;
    struct v4l2_cropcap cropcap;
    struct v4l2_crop crop;
    struct v4l2_format fmt;

    unsigned int min;

    if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &cap))
    {
        if (EINVAL == errno)
        {
            fprintf(stderr, "%s is no V4L2 device\n",deviceName);
            exit(EXIT_FAILURE);
        } else
        {
            errno_exit("VIDIOC_QUERYCAP");
        }
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
    {
        fprintf(stderr, "%s is no video capture device\n",deviceName);
        exit(EXIT_FAILURE);
    }

    struct v4l2_input input;
    input.index = 0;
    if ( ioctl(fd, VIDIOC_ENUMINPUT, &input) != 0)
    {
        printf(" set input error.\n");
        exit(0);
    }

    if ((ioctl(fd, VIDIOC_S_INPUT, &input)) < 0)
    {
        printf("set s_input error.\n");
        exit(0);
    }

    /* Select video input, video standard and tune here. */
    CLEAR(cropcap);

    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (0 == xioctl(fd, VIDIOC_CROPCAP, &cropcap))
    {
          crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
          crop.c = cropcap.defrect; /* reset to default */

          if (-1 == xioctl(fd, VIDIOC_S_CROP, &crop))
          {
              switch (errno)
              {
                      case EINVAL:
              /* Cropping not supported. */
                              break;
                      default:
               /* Errors ignored. */
                              break;
              }
          }
      }
    else
    {
        /* Errors ignored. */
    }

    CLEAR (fmt);

    // v4l2_format
    fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width       = width;
    fmt.fmt.pix.height      = height;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB32;
    // 	  fmt.fmt.pix.field       = V4L2_FIELD_ANY;

    if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt))
          errno_exit("VIDIOC_S_FMT");

    /* Note VIDIOC_S_FMT may change width and height. */
    if (width != fmt.fmt.pix.width) {
          width = fmt.fmt.pix.width;
          fprintf(stderr,"Image width set to %i by device %s.\n",width,deviceName);
    }
    if (height != fmt.fmt.pix.height) {
          height = fmt.fmt.pix.height;
          fprintf(stderr,"Image height set to %i by device %s.\n",height,deviceName);
    }

    /* Buggy driver paranoia. */
    min = fmt.fmt.pix.width * 2;
    if (fmt.fmt.pix.bytesperline < min)
          fmt.fmt.pix.bytesperline = min;

    min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;

    if (fmt.fmt.pix.sizeimage < min)
          fmt.fmt.pix.sizeimage = min;


    mmapInit();
}

void mmapInit(void)
{
    struct v4l2_requestbuffers req;

    CLEAR (req);

    req.count               = 4;
    req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory              = V4L2_MEMORY_MMAP;

    if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req))
    {
        if (EINVAL == errno)
        {
            fprintf(stderr, "%s does not support memory mapping\n", deviceName);
            exit(EXIT_FAILURE);
        }
        else
        {
            errno_exit("VIDIOC_REQBUFS");
        }
    }

    if (req.count < 2)
    {
        fprintf(stderr, "Insufficient buffer memory on %s\n", deviceName);
        exit(EXIT_FAILURE);
    }

    buffers1 = (struct buffer*)calloc(req.count, sizeof(*buffers1));

    if (!buffers1)
    {
        fprintf(stderr, "Out of memory\n");
        exit(EXIT_FAILURE);
    }

    for (n_buffers = 0; n_buffers < req.count; ++n_buffers)
    {
        struct v4l2_buffer buf;

        CLEAR (buf);

        buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory      = V4L2_MEMORY_MMAP;
        buf.index       = n_buffers;

        if (-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf))
            errno_exit("VIDIOC_QUERYBUF");

        buffers1[n_buffers].length = buf.length;

        buffers1[n_buffers].start =
        mmap (NULL /* start anywhere */, buf.length, PROT_READ | PROT_WRITE /* required */, MAP_SHARED /* recommended */, fd, buf.m.offset);

        if (MAP_FAILED == buffers1[n_buffers].start)
            errno_exit("mmap");
    }
}

void captureStart(void)
{
    unsigned int i;
    enum v4l2_buf_type type;

    for (i = 0; i < n_buffers; ++i)
    {
        struct v4l2_buffer buf;

        CLEAR (buf);
        buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory      = V4L2_MEMORY_MMAP;
        buf.index       = i;

        if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
            return;//errno_exit("VIDIOC_QBUF");
    }

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))  //debug by liaoxp 2013-11-27
        errno_exit("VIDIOC_STREAMON");
}

void mainLoop(unsigned char* src)
{
    unsigned int count;
    count = 1;

    while (count-- > 0) {
            for (;;) {
                    fd_set fds;
                    struct timeval tv;
                    int r;

                    FD_ZERO(&fds);
                    FD_SET(fd, &fds);

                    /* Timeout. */
                    tv.tv_sec = 2;
                    tv.tv_usec = 0;

                    r = select(fd + 1, &fds, NULL, NULL, &tv);

                    if (-1 == r) {
                            if (EINTR == errno)
                                    continue;
                            return;//errno_exit("select");
                    }

            if (0 == r) {
                    fprintf (stderr, "select timeout\n");
                    return;//exit(EXIT_FAILURE);
            }
            if (frameRead(src))
                    break;
        }
    }
}

unsigned char* frameRead(unsigned char* src)
{
    struct v4l2_buffer buf;

    CLEAR (buf);


    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

   if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf)) {
            switch (errno) {
                    case EAGAIN:
                            return 0;

                    case EIO:
                    default:
                            return 0;//errno_exit("VIDIOC_DQBUF");
            }
    }
    assert (buf.index < n_buffers);

   imageProcess((void *)buffers1[buf.index].start,src);
     // printf("frameR  = %d\n",buffers1[buf.index].start);

      if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
            return 0;//errno_exit("VIDIOC_QBUF");

  //return (unsigned char*)n;
  //    return n;
}

unsigned char* imageProcess(const void* p,unsigned char* dst)
{
    unsigned short i;
    unsigned char* src = (unsigned char*)p;

   memcpy(dst,bmp_h,54);
    //memcpy(dst+54,src,800*480*4);//800*480*4

    for(i=0; i<20; i++)
    {
        memcpy(dst+54+(i*76800), src+(i*76800), 76800); //debug by liaoxp 2013-11-28
        //memcpy(dst+(i*76800), src, 76800);
    }


    return (unsigned char*)image_buf;
}

void deviceUninit(void)
{
    unsigned int i;

    for (i = 0; i < n_buffers; ++i)
    {
        if (-1 == munmap (buffers1[i].start, buffers1[i].length))
            return;//errno_exit("munmap");
    }

    free(buffers1);
    close(fd);
}

void errno_exit(const char* s)
{
        fprintf(stderr, "%s error %d, %s\n", s, errno, strerror (errno));
       return;// exit(EXIT_FAILURE);
}
int xioctl(int fd, int request, void* argp)
{
        int r;
        do r = ioctl(fd, request, argp);
        while (-1 == r && EINTR == errno);

        return r;
}


/**********************/
/*** 我是传说中的分割线 ***/
/**********************/

/* 以下是CMOS摄像头处理代码 */





