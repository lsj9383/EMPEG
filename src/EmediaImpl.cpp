#include "EmediaImpl.h"

#include<iostream>
#include<fstream>
#include<hash_map>
using namespace std;
//--ffmpeg���ͷ�ļ�
#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"swscale.lib")
#pragma comment(lib, "avutil.lib")

EmediaImpl::EmediaImpl(const std::string& path){
	__filePath = path;
	_videoStream = -1;
	_audioStream = -1;
	_flag = -1;
	//formatCtx->filename
	//hash_map<AVCodecID, VideoType> _videoTypeMap;
	/*_videoTypeMap[AV_CODEC_ID_H264] = H264;
	_videoTypeMap[AV_CODEC_ID_MPEG4] = MPEG4;
	_videoTypeMap[AV_CODEC_ID_JPEG2000] = JPEG2000;*/
}

//�����Ƿ�ɹ�
bool EmediaImpl::_open_(){
	av_register_all();								//��ʼ����װ												
	avformat_network_init();						//��ʼ������� �����Դ�rtsp rtmp http Э�����ý����Ƶ��
	avcodec_register_all();							//ע�������
	AVDictionary *opts = NULL;						//��������
	av_dict_set(&opts, "rtsp_transport", "tcp", 0); //����rtsp����tcpЭ���
	av_dict_set(&opts, "max_delay", "500", 0);		//������ʱʱ��

	const char *pathTemp = __filePath.c_str();
	_flag = avformat_open_input(&_formatCtx, pathTemp, 0, &opts);

	if (_flag != 0){
		char buf[1024] = { 0 };		//��Ŵ�����Ϣ
		av_strerror(_flag, buf, sizeof(buf)-1);
		//cout << "open " << __filePath << " failed! :" << buf << endl;
		//return false;
		throw EmediaException(buf);
	}

	if (avformat_find_stream_info(_formatCtx, 0) < 0){
		//cout<<"Failed to retrieve input stream information\n";
		//return false;
		throw EmediaException("find stream fail call avformat_find_stream_info");
	}

	//--����Ƶ������Ƶ����׼
	_videoStream = av_find_best_stream(_formatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	_audioStream = av_find_best_stream(_formatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);

	//--����Ƶ��������������
	for (int i = 0; i < _formatCtx->nb_streams; i++)
	{
		AVCodecContext *enc = _formatCtx->streams[i]->codec;
		if (enc->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			AVCodec *codec = avcodec_find_decoder(enc->codec_id);
			if (!codec){
				//cout << "video code not find\n";
				//return false;
				throw EmediaException("find decoder call avcodec_find_decoder");
			}
			int err = avcodec_open2(enc, codec, NULL);
			if (err != 0){
				char buf[1024] = { 0 };
				av_strerror(err, buf, sizeof(buf));
				//cout << buf << endl;	return 0;
				throw EmediaException(buf);
			}
			cout << "open codec success by call XFFmpeg::open function\n";
		}
	}
	return true;
}


void EmediaImpl::func1(){
	int ret = -1;
	AVFormatContext *ofmt_ctx;
	AVStream *in_stream = _formatCtx->streams[_videoStream];
	AVStream *out_stream = NULL;

	if (_formatCtx->streams[_videoStream]->codecpar->codec_type != AVMEDIA_TYPE_VIDEO){
		throw EmediaException("error _videoStream!=AVMEDIA_TYPE_VIDEO by  call xvideo in ");
	}
	out_stream = avformat_new_stream(_ofmt_ctx_v, in_stream->codec->codec);
	ofmt_ctx = _ofmt_ctx_v;

	if (!out_stream) {
		printf("Failed allocating output stream\n");
		ret = AVERROR_UNKNOWN;
		goto end;
	}
	//Copy the settings of AVCodecContext
	if (avcodec_copy_context(out_stream->codec, in_stream->codec) < 0) {
		printf("Failed to copy context from input to output stream codec context\n");
		goto end;
	}
	out_stream->codec->codec_tag = 0;

	if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
		out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;

}
bool EmediaImpl::xvideo(const std::string& path){
	AVOutputFormat  *ofmt_v = NULL;
	
	AVPacket pkt;
	int ret = 0, i = 0;
	int frame_index = 0;

	//Output---ofmt_ctx_v������ļ���ʽ
	const char* out_filename_v = path.c_str();

	avformat_alloc_output_context2(&ofmt_ctx_v, NULL, NULL, out_filename_v);
	if (!ofmt_ctx_v) {
		printf("Could not create output context\n");
		ret = AVERROR_UNKNOWN;
		goto end;
	}
	ofmt_v = ofmt_ctx_v->oformat;

	//--------------------------------------------------------------------	
	func1();

	//Open output file
	if (!(ofmt_v->flags & AVFMT_NOFILE)) {
		if (avio_open(&ofmt_ctx_v->pb, out_filename_v, AVIO_FLAG_WRITE) < 0) {
			printf("Could not open output file '%s'", out_filename_v);
			goto end;
		}
	}

	//Write file header,������
	if (avformat_write_header(ofmt_ctx_v, NULL) < 0) {
		printf("Error occurred when opening video output file\n");
		goto end;
	}

#if USE_H264BSF
	AVBitStreamFilterContext* h264bsfc = av_bitstream_filter_init("h264_mp4toannexb");
#endif

	while (1)
	{
		AVFormatContext *ofmt_ctx;
		AVStream *in_stream, *out_stream;

		if (av_read_frame(_formatCtx, &pkt) < 0)
			break;
		in_stream = _formatCtx->streams[pkt.stream_index];

		if (pkt.stream_index == _videoStream){
			out_stream = ofmt_ctx_v->streams[0];
			ofmt_ctx = ofmt_ctx_v;
			printf("Write Video Packet. size:%d\tpts:%lld\n", pkt.size, pkt.pts);
#if USE_H264BSF
			av_bitstream_filter_filter(h264bsfc, in_stream->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);
#endif
		}
		else	continue;

		//Convert PTS/DTS
		pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
		pkt.pos = -1;
		pkt.stream_index = 0;
		//Write
		if (av_interleaved_write_frame(ofmt_ctx, &pkt) < 0) {
			printf("Error muxing packet\n");
			return false;
		}

		av_free_packet(&pkt);
		frame_index++;
	}

#if USE_H264BSF
	av_bitstream_filter_close(h264bsfc);
#endif
	//Write file trailer
	av_write_trailer(ofmt_ctx_v);
end:
	avformat_close_input(&_formatCtx);
	/* close output */
	if (ofmt_ctx_v && !(ofmt_v->flags & AVFMT_NOFILE))
		avio_close(ofmt_ctx_v->pb);

	avformat_free_context(ofmt_ctx_v);

	if (ret < 0 && ret != AVERROR_EOF) {
		printf("Error occurred.\n");
		return false;
	}
	cout << "---------xvideo end-----\n";
	return true;
}














//----��ȡ��Ƶ����Ƶ
bool EmediaImpl::demuxer(const std::string& videoPath, const std::string& audioPath){
	AVOutputFormat *ofmt_a = NULL, *ofmt_v = NULL;
	//��Input AVFormatContext and Output AVFormatContext��
	AVFormatContext *ofmt_ctx_a = NULL, *ofmt_ctx_v = NULL;
	AVPacket pkt;
	int ret = 0, i = 0;
	int videoindex = -1, audioindex = -1;
	int frame_index = 0;

	//Output---ofmt_ctx_v������ļ���ʽ
	const char* out_filename_v = videoPath.c_str();
	const char* out_filename_a = audioPath.c_str();
	avformat_alloc_output_context2(&ofmt_ctx_v, NULL, NULL, out_filename_v);
	if (!ofmt_ctx_v) {
		printf("Could not create output context\n");
		ret = AVERROR_UNKNOWN;
		goto end;
	}
	ofmt_v = ofmt_ctx_v->oformat;

	avformat_alloc_output_context2(&ofmt_ctx_a, NULL, NULL, out_filename_a);
	if (!ofmt_ctx_a) {
		printf("Could not create output context\n");
		ret = AVERROR_UNKNOWN;
		goto end;
	}
	ofmt_a = ofmt_ctx_a->oformat;
	//--------------------------------------------------------------------	
	for (i = 0; i < _formatCtx->nb_streams; i++)
	{
		//Create output AVStream according to input AVStream		
		AVFormatContext *ofmt_ctx;
		AVStream *in_stream = _formatCtx->streams[i];
		AVStream *out_stream = NULL;
		if (_formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
			videoindex = i;
			out_stream = avformat_new_stream(ofmt_ctx_v, in_stream->codec->codec);
			ofmt_ctx = ofmt_ctx_v;
		}
		else if (_formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO){
			audioindex = i;
			out_stream = avformat_new_stream(ofmt_ctx_a, in_stream->codec->codec);
			ofmt_ctx = ofmt_ctx_a;
		}
		else	break;

		if (!out_stream) {
			printf("Failed allocating output stream\n");
			ret = AVERROR_UNKNOWN;
			goto end;
		}
		//Copy the settings of AVCodecContext
		if (avcodec_copy_context(out_stream->codec, in_stream->codec) < 0) {
			printf("Failed to copy context from input to output stream codec context\n");
			goto end;
		}
		out_stream->codec->codec_tag = 0;

		if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
			out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
	}

	//Open output file
	if (!(ofmt_v->flags & AVFMT_NOFILE)) {
		if (avio_open(&ofmt_ctx_v->pb, out_filename_v, AVIO_FLAG_WRITE) < 0) {
			printf("Could not open output file '%s'", out_filename_v);
			goto end;
		}
	}

	if (!(ofmt_a->flags & AVFMT_NOFILE)) {
		if (avio_open(&ofmt_ctx_a->pb, out_filename_a, AVIO_FLAG_WRITE) < 0) {
			printf("Could not open output file '%s'", out_filename_a);
			goto end;
		}
	}

	//Write file header,������
	if (avformat_write_header(ofmt_ctx_v, NULL) < 0) {
		printf("Error occurred when opening video output file\n");
		goto end;
	}
	if (avformat_write_header(ofmt_ctx_a, NULL) < 0) {
		printf("Error occurred when opening audio output file\n");
		goto end;
	}
#if USE_H264BSF
	AVBitStreamFilterContext* h264bsfc = av_bitstream_filter_init("h264_mp4toannexb");
#endif
	while (1)
	{
		AVFormatContext *ofmt_ctx;
		AVStream *in_stream, *out_stream;
		//Get an AVPacket
		if (av_read_frame(_formatCtx, &pkt) < 0)	break;
		in_stream = _formatCtx->streams[pkt.stream_index];

		if (pkt.stream_index == videoindex){
			out_stream = ofmt_ctx_v->streams[0];
			ofmt_ctx = ofmt_ctx_v;
			printf("Write Video Packet. size:%d\tpts:%lld\n", pkt.size, pkt.pts);
#if USE_H264BSF
			av_bitstream_filter_filter(h264bsfc, in_stream->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);
#endif
		}
		else if (pkt.stream_index == audioindex){
			out_stream = ofmt_ctx_a->streams[0];
			ofmt_ctx = ofmt_ctx_a;
			printf("Write Audio Packet. size:%d\tpts:%lld\n", pkt.size, pkt.pts);
		}
		else	continue;

		//Convert PTS/DTS
		pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
		pkt.pos = -1;
		pkt.stream_index = 0;
		//Write
		if (av_interleaved_write_frame(ofmt_ctx, &pkt) < 0) {
			printf("Error muxing packet\n");
			break;
		}
		//printf("Write %8d frames to output file\n",frame_index);
		av_free_packet(&pkt);
		frame_index++;
	}

#if USE_H264BSF
	av_bitstream_filter_close(h264bsfc);
#endif

	//Write file trailer
	av_write_trailer(ofmt_ctx_a);
	av_write_trailer(ofmt_ctx_v);
end:
	avformat_close_input(&_formatCtx);
	/* close output */
	if (ofmt_ctx_a && !(ofmt_a->flags & AVFMT_NOFILE))
		avio_close(ofmt_ctx_a->pb);

	if (ofmt_ctx_v && !(ofmt_v->flags & AVFMT_NOFILE))
		avio_close(ofmt_ctx_v->pb);

	avformat_free_context(ofmt_ctx_a);
	avformat_free_context(ofmt_ctx_v);

	if (ret < 0 && ret != AVERROR_EOF) {
		printf("Error occurred.\n");
		return false;
	}
	return true;
}

bool EmediaImpl::xaudio(const std::string& path){
	AVOutputFormat *ofmt_a = NULL;

	AVFormatContext *ofmt_ctx_a = NULL;
	AVPacket pkt;
	int ret = 0, i = 0;
	int audioindex = -1;
	int frame_index = 0;

	//Output---ofmt_ctx_v������ļ���ʽ
	const char* out_filename_a = path.c_str();

	avformat_alloc_output_context2(&ofmt_ctx_a, NULL, NULL, out_filename_a);
	if (!ofmt_ctx_a) {
		printf("Could not create output context\n");
		ret = AVERROR_UNKNOWN;
		goto end;
	}
	ofmt_a = ofmt_ctx_a->oformat;

	//--------------------------------------------------------------------	
	for (i = 0; i < _formatCtx->nb_streams; i++)
	{
		//Create output AVStream according to input AVStream		
		AVFormatContext *ofmt_ctx;
		AVStream *in_stream = _formatCtx->streams[i];
		AVStream *out_stream = NULL;
		if (_formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
			continue;
		}
		else if (_formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO){
			audioindex = i;
			out_stream = avformat_new_stream(ofmt_ctx_a, in_stream->codec->codec);
			ofmt_ctx = ofmt_ctx_a;
		}
		else{
			break;
		}

		if (!out_stream) {
			printf("Failed allocating output stream\n");
			ret = AVERROR_UNKNOWN;
			goto end;
		}
		//Copy the settings of AVCodecContext
		if (avcodec_copy_context(out_stream->codec, in_stream->codec) < 0) {
			printf("Failed to copy context from input to output stream codec context\n");
			goto end;
		}
		out_stream->codec->codec_tag = 0;

		if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
			out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
	}

	if (!(ofmt_a->flags & AVFMT_NOFILE)) {
		if (avio_open(&ofmt_ctx_a->pb, out_filename_a, AVIO_FLAG_WRITE) < 0) {
			printf("Could not open output file '%s'", out_filename_a);
			goto end;
		}
	}

	//Write file header,������
	if (avformat_write_header(ofmt_ctx_a, NULL) < 0) {
		printf("Error occurred when opening audio output file\n");
		goto end;
	}

	while (1)
	{
		AVFormatContext *ofmt_ctx;
		AVStream *in_stream, *out_stream;

		if (av_read_frame(_formatCtx, &pkt) < 0)
			break;
		in_stream = _formatCtx->streams[pkt.stream_index];


		if (pkt.stream_index == audioindex){
			out_stream = ofmt_ctx_a->streams[0];
			ofmt_ctx = ofmt_ctx_a;
			printf("Write Audio Packet. size:%d\tpts:%lld\n", pkt.size, pkt.pts);
		}
		else{
			continue;
		}
		//Convert PTS/DTS
		pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
		pkt.pos = -1;
		pkt.stream_index = 0;
		//Write
		if (av_interleaved_write_frame(ofmt_ctx, &pkt) < 0) {
			printf("Error muxing packet\n");
			break;
		}

		av_free_packet(&pkt);
		frame_index++;
	}

	av_write_trailer(ofmt_ctx_a);
end:
	avformat_close_input(&_formatCtx);
	/* close output */
	if (ofmt_ctx_a && !(ofmt_a->flags & AVFMT_NOFILE))
		avio_close(ofmt_ctx_a->pb);

	avformat_free_context(ofmt_ctx_a);

	if (ret < 0 && ret != AVERROR_EOF) {
		printf("Error occurred.\n");
		return false;
	}
	cout << "-----xaudio end--------\n";
	return true;
}

bool EmediaImpl::_read_frame(AVPacket& pkt){
	char errorbuf[512] = { 0 };
	int err = av_read_frame(_formatCtx, &pkt);
	if (err != 0){
		av_strerror(err, errorbuf, sizeof(errorbuf));
		throw EmediaException(errorbuf);
	}
	//memset(pkt, 0, sizeof(AVPacket));
	return true;
}

//----����һ��packet
bool EmediaImpl::_decode(AVPacket* pkt, AVFrame& yuv){
	int re = avcodec_send_packet(_formatCtx->streams[pkt->stream_index]->codec, pkt);	//�漰������
	if (re != 0){
		//cout << "error in avcodec_send_packet\n";
		//return false;
		throw EmediaException("error in avcodec_send_packet");
	}
	re = avcodec_receive_frame(_formatCtx->streams[pkt->stream_index]->codec, &yuv);
	if (re != 0){
		//cout << "error in avcodec_receive_frame\n";
		//return false;
		throw EmediaException("error in avcodec_send_packet");
	}
	return true;
}

//--��ȡyuv
bool EmediaImpl::xyuv(const std::string& path){
	//-��������frame��YUV240�ĸ�ʽд���ļ�
	//	AVFrame* pFrame;
	ofstream ofile(path, ios::binary);	//yuv�ļ�
	if (!ofile){
		cerr << "open out file of yuv error\n";
		abort();
	}

	AVFrame* pFrameYUV = av_frame_alloc();
	uint8_t* out_buffer;
	AVCodecContext* pCodecCtx = _formatCtx->streams[_videoStream]->codec;
	out_buffer = (unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1));
	av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);

	//���ø�ʽ
	//out_buffer = new uint8_t[avpicture_get_size(PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height)];
	//avpicture_fill( (AVPicture *)pFrameYUV, out_buffer, PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);

	//--ת��
	struct SwsContext *img_convert_ctx = NULL;
	img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);

	while (1)
	{
		AVPacket* pkt = av_packet_alloc();
		AVFrame* frame = av_frame_alloc();

		//_read_frame(*pkt);
		int err = av_read_frame(_formatCtx, pkt);
		if (pkt->size == 0){
			cout << "------------����ȫ����pkt---------\n";
			break;
		}
		//������Ƶpacket,*********************************
		if (pkt->stream_index != _videoStream){
			av_packet_unref(pkt);	//�ͷſռ� 
			continue;
		}

		EmediaImpl::_decode(pkt, *frame);
		av_packet_unref(pkt);	//�ͷſռ� 

		//-------------------------------------------------------------------
		sws_scale(img_convert_ctx, (const uint8_t* const*)frame->data, frame->linesize, 0,
			pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);
		//дYUV
		ofile.write((char*)pFrameYUV->data[0], (pCodecCtx->width)*(pCodecCtx->height));
		ofile.write((char*)pFrameYUV->data[1], (pCodecCtx->width)*(pCodecCtx->height) / 4);
		ofile.write((char*)pFrameYUV->data[2], (pCodecCtx->width)*(pCodecCtx->height) / 4);
		cout << "--------------write----------------\n";
	}

	ofile.clear();
	return true;
}




















// ֻ������
const string& EmediaImpl::where(){
	return __filePath;
}

int EmediaImpl::high(){
	return _formatCtx->streams[_videoStream]->codecpar->height;
}
int EmediaImpl::width(){
	return _formatCtx->streams[_videoStream]->codecpar->width;
}

int64_t EmediaImpl::frames(){
	return _formatCtx->streams[_videoStream]->nb_frames;
}

double EmediaImpl::fps(){
	AVRational R = _formatCtx->streams[_videoStream]->avg_frame_rate;
	return R.num == 0 | R.den == 0 ? 0.0 : (double)R.num / (double)R.den;
}

VideoType EmediaImpl::video_type(){
	//if (_videoTypeMap.find(_formatCtx->streams[_videoStream]->codecpar->codec_id) != _videoTypeMap.end)
	//	return _videoTypeMap[_formatCtx->streams[_videoStream]->codecpar->codec_id];
	//else
	return NONE;
}