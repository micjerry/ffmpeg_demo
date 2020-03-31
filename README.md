Makefile为样例文件，每个demo需要修改一下，Demo编译过程如下：  

cp ffmpeg/doc/examples/Makefile.example tests/Makefile  
cp ffm_config.c tests  
cd tests  
make  
./ffm_config  
  
ffmpeg自带Demo介绍：  
  
avio_reading.c 读取媒体文件，获取基本信息  
decode_audio.c 解码音频文件  
encode_audio.c 生成随机数据并编码成音频包  
encode_video.c 生成随机数据并编码成视频包  
demuxing_decoding.c 将媒体文件分离成视频和音频 ./demuxing_decoding sample.avi sample.yuv sample.pcm   
filtering_audio.c 音频处理过滤器，可以对音频进行各种处理包括调整音量、压缩、延迟、回声效果、合成、检测静音、移除静音、生成噪声等  
filtering_video.c 视频过滤器，可以对视频进行各种处理包括边缘检测、水平垂直镜像、视频叠加、边界填充、任意角旋转、缩放、添加字幕等。
  
参考改写Demo介绍：  
  
decode_h264_to_pgm.c：解码H.264保存成pgm格式，每帧一张图 ./decode_video demo_video.h264 output.pgm  
decode_h264_to_yuv.c：解码H.264保存成yuv格式，./decode_video output.h264 output.yuv  
decode_h264_to_oneyuv.c 解码H.264保存成一个yuv文件  ./decode_video output.h264 output.yuv
encode_yuv_to_h264.c：编码yuv为H.264格式，./encode_video ../simplest_decoder/output.yuv output.h264 libx264  
decode_mp2_to_pcm.c：解码音频mp2为pcm格式，./decode_audio sample-96kbps.mp2 sample.pcm  
encode_pcm_to_mp2.c：编码pcm为mp2格式，./encode_audio sample.pcm sample.mp2  
remuxing.c: 封装格式转换，封装格式是指avi、flv、mkv、mp4等，封装格式转换并不需要把音视频重新解码编码  
demuxing_decoding.c：解码并分离音视频，./demuxing_decoding sample.avi sample.yuv sample.pcm  
