#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libavcodec/avcodec.h>

#include <libavutil/opt.h>
#include <libavutil/imgutils.h>

static void encode(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *pkt,
                   FILE *outfile)
{
    int ret;

    /* send the frame to the encoder */
    if (frame)
        printf("Send frame %3"PRId64"\n", frame->pts);

    ret = avcodec_send_frame(enc_ctx, frame);
    if (ret < 0) {
        fprintf(stderr, "Error sending a frame for encoding\n");
        exit(1);
    }

    while (ret >= 0) {
        ret = avcodec_receive_packet(enc_ctx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0) {
            fprintf(stderr, "Error during encoding\n");
            exit(1);
        }

        printf("Write packet %3"PRId64" (size=%5d)\n", pkt->pts, pkt->size);
        fwrite(pkt->data, 1, pkt->size, outfile);
        av_packet_unref(pkt);
    }
}
int main(int argc, char **argv)
{
    const char *filename, *codec_name, *input_filename;
    const AVCodec *codec;
    AVCodecContext *c= NULL;
    int i, ret;
    FILE *f;
    FILE *in_file;
    AVFrame *frame;
    AVPacket *pkt;
    uint8_t* picture_buf;
    int picture_size;
    int y_size;
    uint8_t endcode[] = { 0, 0, 1, 0xb7 };

    if (argc <= 2) {
        fprintf(stderr, "Usage: %s <output file> <codec name>\n", argv[0]);
        exit(0);
    }
    input_filename = argv[1];
    filename = argv[2];
    codec_name = argv[3];

    in_file = fopen(input_filename, "rb");

    /* find the mpeg1video encoder */
    codec = avcodec_find_encoder_by_name(codec_name);
    if (!codec) {
        fprintf(stderr, "Codec '%s' not found\n", codec_name);
        exit(1);
    }

    c = avcodec_alloc_context3(codec);
    if (!c) {
        fprintf(stderr, "Could not allocate video codec context\n");
        exit(1);
    }

    pkt = av_packet_alloc();
    if (!pkt)
        exit(1);

    /* put sample parameters */
    c->bit_rate = 4000000;
    /* resolution must be a multiple of two */
    c->width = 1024;
    c->height = 576;
    /* frames per second */
    c->time_base = (AVRational){1, 25};
    c->framerate = (AVRational){25, 1};
    /* emit one intra frame every ten frames
     * check frame pict_type before passing frame
     * to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
     * then gop_size is ignored and the output of encoder
     * will always be I frame irrespective to gop_size
     */
    c->gop_size = 10;
    c->max_b_frames = 1;
    c->pix_fmt = AV_PIX_FMT_YUV420P;

    if (codec->id == AV_CODEC_ID_H264)
        av_opt_set(c->priv_data, "preset", "slow", 0);

    /* open it */
    ret = avcodec_open2(c, codec, NULL);
    if (ret < 0) {
        fprintf(stderr, "Could not open codec: %s\n", av_err2str(ret));
        exit(1);
    }

    f = fopen(filename, "wb");
    if (!f) {
        fprintf(stderr, "Could not open %s\n", filename);
        exit(1);
    }

    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate video frame\n");
        exit(1);
    }
    frame->format = c->pix_fmt;
    frame->width  = c->width;
    frame->height = c->height;

    ret = av_frame_get_buffer(frame, 32);
    if (ret < 0) {
        fprintf(stderr, "Could not allocate the video frame data\n");
        exit(1);
    }

    y_size = c->width * c->height;
    picture_size = av_image_get_buffer_size(c->pix_fmt, c->width, c->height, 1);
    picture_buf = (uint8_t *)av_malloc(picture_size);

    for (i = 0; i < 199; i++) {
        fflush(stdout);

        ret = av_frame_make_writable(frame);
        if (ret < 0)
            exit(1);

        if (fread(picture_buf, 1, y_size*3/2, in_file) <= 0){
            printf("Failed to read raw data! \n");
            return -1;
        } else if(feof(in_file)){
            break;
        }

        frame->data[0] = picture_buf;
        frame->data[1] = picture_buf + y_size;
        frame->data[2] = picture_buf + y_size*5/4;

        frame->pts = i;

        /* encode the image */
        encode(c, frame, pkt, f);
    }

    /* flush the encoder */
    encode(c, NULL, pkt, f);

    /* add sequence end code to have a real MPEG file */
    fwrite(endcode, 1, sizeof(endcode), f);
    fclose(f);
    fclose(in_file);

    avcodec_free_context(&c);
    av_frame_free(&frame);
    av_packet_free(&pkt);

    return 0;
}