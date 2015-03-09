#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <gst/app/gstappsrc.h>
#include <gst/app/gstappbuffer.h>
#include <gst/app/gstappsink.h>  
#include <linux/videodev2.h>
#include <linux/mxc_v4l2.h>
#include <sys/ioctl.h>
#include <opencv2/opencv.hpp>
#include "opencv2/video/background_segm.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

int fd_video;
int shutter;
int gain;
GMainLoop *loop;

    GstBus *bus;
    GstMessage *msg;
    GstStateChangeReturn ret;
    GstElement *pipeline_v1, *pipeline_v2;
    GstElement *source, *sink_app, *sink_file, *csp, *enc, *videotee, *parser,*mart;
    GstElement *queue1, *queue2;
    GstElement *rtp, *udpsink, *fakesink;


    
typedef struct _CustomData {
    gboolean is_live;
    GstElement *pipeline;
    GMainLoop *loop;
} CustomData;


void set_exposure(int exp) {
	v4l2_control par_exp;
	par_exp.id=V4L2_CID_EXPOSURE;
	par_exp.value=exp;
	if (ioctl(fd_video, VIDIOC_S_CTRL, &par_exp) < 0)
	{
		printf("\nVIDIOC_S_CTRL failed\n");
	} else {
	 printf("shutter set %d \n",par_exp.value);
	}
}


void set_gain(int gain) {
	v4l2_control par_exp;
	par_exp.id=V4L2_CID_GAIN;
	par_exp.value=gain;
	if (ioctl(fd_video, VIDIOC_S_CTRL, &par_exp) < 0)
	{
		printf("\nVIDIOC_S_CTRL failed\n");
	} else {
	 printf("gain set %d \n",par_exp.value);
	}
}

void print_buffer (GstBuffer *buffer, const char *title) {
    
    GstCaps *caps = gst_buffer_get_caps(buffer);
    for (uint j = 0; j < gst_caps_get_size(caps); ++j) {
        GstStructure *structure = gst_caps_get_structure(caps, j);
        printf("%s{%ss}: ", (title), (gst_structure_get_name(structure)));
        for (int i = 0; i < gst_structure_n_fields(structure); ++i) {
            if (i != 0)
                printf(", ");
            const char *name = gst_structure_nth_field_name(structure, i);
            GType type = gst_structure_get_field_type(structure, name);
            printf("%s[%s]", (name), (g_type_name(type)));
        }
        printf("\n");
    }
    
}

static gboolean bus_call(GstBus *bus, GstMessage *msg, gpointer user_data) {
    int i;
    
    switch (GST_MESSAGE_TYPE(msg)) {
        case GST_MESSAGE_STATE_CHANGED:
        {
            GstState old_state, new_state;

            gst_message_parse_state_changed(msg, &old_state, &new_state, NULL);
            printf("[%s]: %s -> %s\n", GST_OBJECT_NAME(msg->src), gst_element_state_get_name(old_state), gst_element_state_get_name(new_state));
            if (strcmp(msg->src->name,"video")==0) {
                if (strcmp(gst_element_state_get_name(new_state),"READY")==0) {
                    g_object_get (G_OBJECT (source), "file-id", &fd_video, NULL);
		    set_exposure(shutter);
 		    set_gain(gain);
                    printf("video file %d \n",fd_video);
                }
            }
            break;
        }
        case GST_MESSAGE_ERROR:
        {
            gchar *debug;
            GError *err;

            gst_message_parse_error(msg, &err, &debug);
            printf("[%s]: %s %s\n", GST_OBJECT_NAME(msg->src), err->message, debug);
            g_free(debug);
            g_error_free(err);
            exit(1);
            break;
        }
        case GST_MESSAGE_EOS:
            /* end-of-stream */
            g_main_loop_quit(loop);
            printf("finished record\n");
            break;
        default:
        {
/*            
            const GstStructure *structure = gst_message_get_structure(msg);
            if (structure) {
                printf("%s{%s} ", gst_message_type_get_name(msg->type), gst_structure_get_name(structure));
                for (i = 0; i < gst_structure_n_fields(structure); ++i) {
                    if (i != 0) printf(", ");
                    const char *name = gst_structure_nth_field_name(structure, i);
                    GType type = gst_structure_get_field_type(structure, name);
                    printf("%s", name);
                    printf("[%s]", g_type_name(type));
                }
            printf("\n");
            } else {
                printf("info: %i %s type: %i\n", (int) (msg->timestamp), GST_MESSAGE_TYPE_NAME(msg), msg->type);
                 printf("%s{}\n", gst_message_type_get_name (msg->type));
            }
*/
            break;
        }
    }
    return true;
}

void add_cliden (GstElement* object, gchararray arg0, gint arg1, gpointer user_data){
    printf("add clien \n");
}

void almost_c99_signal_handler(int signum)
{
 switch(signum)
  {
    case SIGABRT:
      fputs("Caught SIGABRT: usually caused by an abort() or assert()\n", stderr);
      break;
    case SIGFPE:
      fputs("Caught SIGFPE: arithmetic exception, such as divide by zero\n",
            stderr);
      break;
    case SIGILL:
      fputs("Caught SIGILL: illegal instruction\n", stderr);
      break;
    case SIGINT:
      fputs("Caught SIGINT: interactive attention signal, probably a ctrl+c!!!!\n",
            stderr);
      g_main_loop_quit(loop);
      return ;
      break;
    case SIGSEGV:
      fputs("Caught SIGSEGV: segfault\n", stderr);
      break;
    case SIGTERM:
    default:
      fputs("Caught SIGTERM: a termination request was sent to the program\n",
            stderr);
      break;
  }
  _Exit(1);
}


int main(int argc, char *argv[]) {

    /* Initialize GStreamer */
    signal(SIGABRT, almost_c99_signal_handler);
    signal(SIGFPE,  almost_c99_signal_handler);
    signal(SIGILL,  almost_c99_signal_handler);
    signal(SIGINT,  almost_c99_signal_handler);
    signal(SIGSEGV, almost_c99_signal_handler);
    signal(SIGTERM, almost_c99_signal_handler);
    
    gst_init(&argc, &argv);

    if (argc<3) {
        printf("command must have four arguments: file , shutte, gain and number of frames example (name.avi 100 10 300).\n");
        return 0;
    }

    shutter=atoi(argv[2]);
    gain=atoi(argv[3]);
    printf("Start Record file %s : shutter %d\n",argv[1],atoi(argv[2]));	

    int numberoffr=atoi(argv[4]);
    source = gst_element_factory_make("imxv4l2src", "video");
//    source = gst_element_factory_make("v4l2src", "video");
    
    queue1 = gst_element_factory_make("queue", "queue");
    //g_object_set( G_OBJECT(queue1), "max-size-buffers", 10, NULL);
        
       enc = gst_element_factory_make("vpuenc", "imxvpu");
 	     g_object_set( G_OBJECT(enc), "codec", 6, NULL); 
//     enc = gst_element_factory_make("ffenc_mpeg4", "ffenc_mpeg4");
     
     mart=gst_element_factory_make("matroskamux","matroskamux");
                                    
     sink_file = gst_element_factory_make("filesink", "filesink");
     g_object_set(G_OBJECT(sink_file), "location", argv[1], NULL);
    
 //  g_object_set(G_OBJECT(source), "device", "/dev/video0", NULL);
     g_object_set( G_OBJECT(source), "num-buffers", numberoffr, NULL);
 //  g_object_set( G_OBJECT(source), "capture-mode", 0, NULL); 
 //    g_object_set( G_OBJECT(source), "fps-n", "30", NULL); 

    pipeline_v1 = gst_pipeline_new("cam-pipeline");
    
    if (!pipeline_v1 || !source  || !sink_file || !queue1 || !enc || !mart) {
        g_printerr("Not pipeline element could be created.\n");
        return -1;
    }

    gst_bin_add_many(GST_BIN(pipeline_v1), source,queue1, enc,mart, sink_file, NULL);
   

    if (!gst_element_link_many(source, queue1, enc, mart,sink_file , NULL)) {
        gst_object_unref(pipeline_v1);
        printf("Errors link many \n");
        exit(1);
    }


    bus = gst_element_get_bus(pipeline_v1);
        
    loop = g_main_loop_new(NULL, FALSE);
/*
    GstAppSinkCallbacks callbacks = { NULL, new_preroll, new_buffer,
                                      new_buffer_list, { NULL } };
    gst_app_sink_set_callbacks (GST_APP_SINK(sink_app), &callbacks, NULL, NULL);
    
    CustomData data;
    data.loop = loop;
    data.pipeline = pipeline_v1;    
*/    
    guint bus_watch_id = gst_bus_add_watch(bus, bus_call, NULL);  
        
    ret = gst_element_set_state(pipeline_v1, GST_STATE_PLAYING);
    
    g_main_loop_run(loop);
    gst_element_set_state(pipeline_v1, GST_STATE_NULL);
    
    gst_object_unref(bus);
    gst_object_unref(pipeline_v1);

    return 0;
}
