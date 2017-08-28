/*******************************************************************************
# Linux-UVC streaming input-plugin for MJPG-streamer                           #
#                                                                              #
# This package work with the Logitech UVC based webcams with the mjpeg feature #
#                                                                              #
# Copyright (C) 2005 2006 Laurent Pinchart &&  Michel Xhaard                   #
#                    2007 Lucas van Staden                                     #
#                    2007 Tom Stöveken                                         #
#                                                                              #
# This program is free software; you can redistribute it and/or modify         #
# it under the terms of the GNU General Public License as published by         #
# the Free Software Foundation; either version 2 of the License, or            #
# (at your option) any later version.                                          #
#                                                                              #
# This program is distributed in the hope that it will be useful,              #
# but WITHOUT ANY WARRANTY; without even the implied warranty of               #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                #
# GNU General Public License for more details.                                 #
#                                                                              #
# You should have received a copy of the GNU General Public License            #
# along with this program; if not, write to the Free Software                  #
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA    #
#                                                                              #
*******************************************************************************/

#include <stdlib.h>
#include "v4l2uvc.h"
#include "huffman.h"
#include "dynctrl.h"

static int debug = 0;
static int init_v4l2(struct vdIn *vd);
static FILE* fdcamera = 0;

int init_videoIn(struct vdIn *vd, char *device, int width, int height, int fps, int format, int grabmethod)
{


  if (vd == NULL || device == NULL)
    return -1;
  if (width == 0 || height == 0)
    return -1;
  if (grabmethod < 0 || grabmethod > 1)
    grabmethod = 1;		//mmap by default;
  vd->videodevice = NULL;
  vd->status = NULL;
  vd->pictName = NULL;
  vd->videodevice = (char *) calloc (1, 16 * sizeof (char));
  vd->status = (char *) calloc (1, 100 * sizeof (char));
  vd->pictName = (char *) calloc (1, 80 * sizeof (char));
  snprintf (vd->videodevice, 12, "%s", device);
  vd->toggleAvi = 0;
  vd->getPict = 0;
  vd->signalquit = 1;
  vd->width = width;
  vd->height = height;
  vd->fps = fps;
  vd->formatIn = format;
  vd->grabmethod = grabmethod;

 
  if (init_v4l2 (vd) < 0) {
    fprintf (stderr, " Init Camera interface failed !! exit fatal \n");
    goto error;;
  }
  
  // alloc a temp buffer to reconstruct the pict 
  vd->framesizeIn = (vd->width * vd->height << 1);
  vd->framebuffer =  (unsigned char *) calloc(1, (size_t) vd->framesizeIn);
  if (!vd->framebuffer)
    goto error;
  return 0;
error:
  free(vd->videodevice);
  free(vd->status);
  free(vd->pictName);
  close(vd->fd);
  return -1;
}

static int init_v4l2(struct vdIn *vd)
{
	int fd = open(vd->videodevice, O_RDONLY);
	if (fd > 0)
	{
		// Set preview width,height
		ioctl(fd, 0x0000, vd->width);
		ioctl(fd, 0x0001, vd->height);
	    	close(fd);
	}
	else
	{
		goto fatal;
	}
	// Open camera
	fdcamera = fopen(vd->videodevice,"rb");

	if (fdcamera == NULL)
	{
		printf("Could not open camera\n");
		goto fatal;
	}

	
  return 0;
fatal:
  return -1;

}

static int video_enable(struct vdIn *vd)
{
	return 0;
/*	
  int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  int ret;

  ret = ioctl(vd->fd, VIDIOC_STREAMON, &type);
  if (ret < 0) {
    perror("Unable to start capture");
    return ret;
  }
  vd->isstreaming = 1;
  return 0;
 */
}

static int video_disable(struct vdIn *vd)
{
	return 0;
/*	
  int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  int ret;

  ret = ioctl(vd->fd, VIDIOC_STREAMOFF, &type);
  if (ret < 0) {
    perror("Unable to stop capture");
    return ret;
  }
  vd->isstreaming = 0;
  return 0;
*/  
}

/******************************************************************************
Description.: 
Input Value.: 
Return Value: 
******************************************************************************/
int is_huffman(unsigned char *buf)
{
  unsigned char *ptbuf;
  int i = 0;
  ptbuf = buf;
  while (((ptbuf[0] << 8) | ptbuf[1]) != 0xffda) {
    if (i++ > 2048)
      return 0;
    if (((ptbuf[0] << 8) | ptbuf[1]) == 0xffc4)
      return 1;
    ptbuf++;
  }
  return 0;
}

/******************************************************************************
Description.: 
Input Value.: 
Return Value: 
******************************************************************************/
int memcpy_picture(unsigned char *out, unsigned char *buf, int size)
{
  unsigned char *ptdeb, *ptlimit, *ptcur = buf;
  int sizein, pos=0;

  if (!is_huffman(buf)) {
    ptdeb = ptcur = buf;
    ptlimit = buf + size;
    while ((((ptcur[0] << 8) | ptcur[1]) != 0xffc0) && (ptcur < ptlimit))
      ptcur++;
    if (ptcur >= ptlimit)
        return pos;
    sizein = ptcur - ptdeb;

    memcpy(out+pos, buf, sizein); pos += sizein;
    memcpy(out+pos, dht_data, sizeof(dht_data)); pos += sizeof(dht_data);
    memcpy(out+pos, ptcur, size - sizein); pos += size-sizein;
  } else {
    memcpy(out+pos, ptcur, size); pos += size;
  }
  return pos;
}

int uvcGrab(struct vdIn *vd)
{
#define HEADERFRAME1 0xaf
  int ret;
  int nSize = 0;
  
  //if (!vd->isstreaming)
  //  if (video_enable(vd))
  //    goto err;

  //memset(&vd->buf, 0, sizeof(struct v4l2_buffer));
  //vd->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  //vd->buf.memory = V4L2_MEMORY_MMAP;

  nSize = fread(vd->framebuffer,1,vd->framesizeIn,fdcamera);
  vd->buf.bytesused = nSize;
  
  //ret = ioctl(vd->fd, VIDIOC_DQBUF, &vd->buf);
  if (nSize !=  vd->framesizeIn) {
    perror("Read size small than requested");
    goto err;
  }


//  ret = ioctl(vd->fd, VIDIOC_QBUF, &vd->buf);
//  if (ret < 0) {
//    perror("Unable to requeue buffer");
//    goto err;
//  }

  return 0;

err:
  vd->signalquit = 0;
  return -1;
}

int close_v4l2(struct vdIn *vd)
{
  //if (vd->isstreaming)
  //  video_disable(vd);
  if (fdcamera != NULL)
  {
  	fclose(fdcamera);
  }
  if (vd->tmpbuffer)
    free(vd->tmpbuffer);
  vd->tmpbuffer = NULL;
  free(vd->framebuffer);
  vd->framebuffer = NULL;
  free(vd->videodevice);
  free(vd->status);
  free(vd->pictName);
  vd->videodevice = NULL;
  vd->status = NULL;
  vd->pictName = NULL;

  return 0;
}

/* return >= 0 ok otherwhise -1 */
static int isv4l2Control(struct vdIn *vd, int control, struct v4l2_queryctrl *queryctrl) {
  int err =0;

  queryctrl->id = control;
  if ((err= ioctl(vd->fd, VIDIOC_QUERYCTRL, queryctrl)) < 0) {
    //fprintf(stderr, "ioctl querycontrol error %d \n",errno);
    return -1;
  }

  if (queryctrl->flags & V4L2_CTRL_FLAG_DISABLED) {
    //fprintf(stderr, "control %s disabled \n", (char *) queryctrl->name);
    return -1;
  }

  if (queryctrl->flags & V4L2_CTRL_TYPE_BOOLEAN) {
    return 1;
  }

  if (queryctrl->type & V4L2_CTRL_TYPE_INTEGER) {
    return 0;
  }

  fprintf(stderr, "contol %s unsupported  \n", (char *) queryctrl->name);
  return -1;
}

int v4l2GetControl(struct vdIn *vd, int control) {
  struct v4l2_queryctrl queryctrl;
  struct v4l2_control control_s;
  int err;

  if ((err = isv4l2Control(vd, control, &queryctrl)) < 0) {
    return -1;
  }

  control_s.id = control;
  if ((err = ioctl(vd->fd, VIDIOC_G_CTRL, &control_s)) < 0) {
    return -1;
  }

  return control_s.value;
}

int v4l2SetControl(struct vdIn *vd, int control, int value) {
  struct v4l2_control control_s;
  struct v4l2_queryctrl queryctrl;
  int min, max, step, val_def;
  int err;

  if (isv4l2Control(vd, control, &queryctrl) < 0)
    return -1;

  min = queryctrl.minimum;
  max = queryctrl.maximum;
  step = queryctrl.step;
  val_def = queryctrl.default_value;

  if ((value >= min) && (value <= max)) {
    control_s.id = control;
    control_s.value = value;
    if ((err = ioctl(vd->fd, VIDIOC_S_CTRL, &control_s)) < 0) {
      return -1;
    }
  }

  return 0;
}

int v4l2UpControl(struct vdIn *vd, int control) {
  struct v4l2_control control_s;
  struct v4l2_queryctrl queryctrl;
  int min, max, current, step, val_def;
  int err;

  if (isv4l2Control(vd, control, &queryctrl) < 0)
    return -1;

  min = queryctrl.minimum;
  max = queryctrl.maximum;
  step = queryctrl.step;
  val_def = queryctrl.default_value;
  if ( (current = v4l2GetControl(vd, control)) == -1 )
    return -1;

  current += step;

  //fprintf(stderr, "max %d, min %d, step %d, default %d ,current %d \n",max,min,step,val_def,current);
  if (current <= max) {
    control_s.id = control;
    control_s.value = current;
    if ((err = ioctl(vd->fd, VIDIOC_S_CTRL, &control_s)) < 0) {
      return -1;
    }
    //fprintf(stderr, "Control name:%s set to value:%d\n", queryctrl.name, control_s.value);
  } else {
    //fprintf(stderr, "Control name:%s already has max value:%d \n", queryctrl.name, max);
    return -1;
  }

  return 0;
}

int v4l2DownControl(struct vdIn *vd, int control) {
  struct v4l2_control control_s;
  struct v4l2_queryctrl queryctrl;
  int min, max, current, step, val_def;
  int err;

  if (isv4l2Control(vd, control, &queryctrl) < 0)
    return -1;

  min = queryctrl.minimum;
  max = queryctrl.maximum;
  step = queryctrl.step;
  val_def = queryctrl.default_value;
  if ( (current = v4l2GetControl(vd, control)) == -1 )
    return -1;

  current -= step;
  //fprintf(stderr, "max %d, min %d, step %d, default %d ,current %d \n",max,min,step,val_def,current);

  if (current >= min) {
    control_s.id = control;
    control_s.value = current;
    if ((err = ioctl(vd->fd, VIDIOC_S_CTRL, &control_s)) < 0) {
      return -1;
    }
    //fprintf(stderr, "Control name:%s set to value:%d\n", queryctrl.name, control_s.value);
  }
  else {
    return -1;
    //fprintf(stderr, "Control name:%s already has min value:%d \n", queryctrl.name, min); 
  }

  return 0;
}

int v4l2ToggleControl(struct vdIn *vd, int control) {
  struct v4l2_control control_s;
  struct v4l2_queryctrl queryctrl;
  int current;
  int err;

  if (isv4l2Control(vd, control, &queryctrl) != 1)
    return -1;

  if ( (current = v4l2GetControl(vd, control)) == -1 )
    return -1;

  control_s.id = control;
  control_s.value = !current;

  if ((err = ioctl(vd->fd, VIDIOC_S_CTRL, &control_s)) < 0) {
    return -1;
  }

  return 0;
}

int v4l2ResetControl(struct vdIn *vd, int control) {
  struct v4l2_control control_s;
  struct v4l2_queryctrl queryctrl;
  int val_def;
  int err;

  if (isv4l2Control(vd, control, &queryctrl) < 0)
    return -1;

  val_def = queryctrl.default_value;
  control_s.id = control;
  control_s.value = val_def;

  if ((err = ioctl(vd->fd, VIDIOC_S_CTRL, &control_s)) < 0) {
    return -1;
  }

  return 0;
}
