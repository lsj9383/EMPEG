/*
#include <iostream>
#include <thread>
extern "C"{
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
}
using namespace std;
#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"swscale.lib")

static double r2d(AVRational r)
{
	return r.den == 0 ? 0 : (double)r.num / (double)r.den;
}
void XSleep(int ms)
{
	//c++ 11
	chrono::milliseconds du(ms);
	this_thread::sleep_for(du);
}
int main(int argc, char *argv[]){
	cout << "Test Demux FFmpeg.club" << endl;
	const char *path = "czl.mp4";

	av_register_all();		//��ʼ����װ��
	
	avformat_network_init();
	avcodec_register_all();			//ע�������
	AVDictionary *opts = NULL;		//��������
	//����rtsp����tcpЭ���
	av_dict_set(&opts, "rtsp_transport", "tcp", 0);

	//������ʱʱ��
	av_dict_set(&opts, "max_delay", "500", 0);

	//-----------���ý���--------------------------------
	//���װ������
	AVFormatContext *ic = NULL;
	int re = avformat_open_input(
		&ic,
		path,
		0,		// 0��ʾ�Զ�ѡ������
		&opts	//�������ã�����rtsp����ʱʱ��
		);
	if (re != 0){
		char buf[1024] = { 0 };		//��Ŵ�����Ϣ
		av_strerror(re, buf, sizeof(buf)-1);
		cout << "open " << path << " failed! :" << buf << endl;
		getchar(); return -1;
	}
	cout << "open " << path << " success! " << endl;

	//--------------------------------------------------	
	re = avformat_find_stream_info(ic, 0);				//��ȡ����Ϣ 
	
	int totalMs = ic->duration / (AV_TIME_BASE / 1000);//��ʱ�� ����
	cout << "totalMs = " << totalMs << endl;

	av_dump_format(ic, 0, path, 0);				//��ӡ��Ƶ����ϸ��Ϣ

	//����Ƶ��������ȡʱ��������Ƶ
	int videoStream = 0; int audioStream = 1;

	//��ȡ����Ƶ����Ϣ(����1������������ȡ)������stream��ȡ����Ƶ��Ϣ
	for (int i = 0; i < ic->nb_streams; i++)
	{
		AVStream *as = ic->streams[i];		//����Ƶ������Ƶ��
		cout << "codec_id = " << as->codecpar->codec_id << endl;
		cout << "format = " << as->codecpar->format << endl;

		//��Ƶ AVMEDIA_TYPE_AUDIO
		if (as->codecpar->codec_type == AVMEDIA_TYPE_AUDIO){
			audioStream = i;
			cout << i << "��Ƶ��Ϣ" << endl;
			cout << "sample_rate = " << as->codecpar->sample_rate << endl;
			//AVSampleFormat;
			cout << "channels = " << as->codecpar->channels << endl;
			//һ֡���ݣ��� ��ͨ�������� 
			cout << "frame_size = " << as->codecpar->frame_size << endl;
			//1024 * 2 * 2 = 4096  fps = sample_rate/frame_size			
		}
		//��Ƶ AVMEDIA_TYPE_VIDEO
		else if (as->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
			videoStream = i;
			cout << i << "��Ƶ��Ϣ" << endl;
			cout << "width=" << as->codecpar->width << endl;
			cout << "height=" << as->codecpar->height << endl;
			//֡�� fps ����ת��
			cout << "video fps = " << r2d(as->avg_frame_rate) << endl;
		}
	}

	//---��ȡ��Ƶ����Ϣ������2��
	videoStream = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);

	//--------------------------------------------------------
	//�ҵ���Ƶ����������Ƶ��������		
	AVCodec *vcodec = avcodec_find_decoder(ic->streams[videoStream]->codecpar->codec_id);
	if (!vcodec){
		cout << "can't find the codec id " << ic->streams[videoStream]->codecpar->codec_id;
		getchar(); return -1;
	}
	cout << "find the AVCodec " << ic->streams[videoStream]->codecpar->codec_id << endl;

	AVCodecContext *vc = avcodec_alloc_context3(vcodec);	//vcodec�����������Ϊ����������vc����ռ�

	///���ý����������Ĳ���
	avcodec_parameters_to_context(vc, ic->streams[videoStream]->codecpar);
	vc->thread_count = 8;		//���߳̽���

	///�򿪽�����������
	re = avcodec_open2(vc, 0, 0);
	if (re != 0){
		char buf[1024] = { 0 };
		av_strerror(re, buf, sizeof(buf)-1);
		cout << "avcodec_open2  failed! :" << buf << endl; getchar(); return -1;
	}
	cout << "video avcodec_open2 success!" << endl;


	//--��Ƶ��������
	//AVCodec *vcodec = avcodec_find_decoder(ic->streams[videoStream]->codecpar->codec_id);//��Ƶ����
	
	AVCodec *acodec = avcodec_find_decoder(ic->streams[audioStream]->codecpar->codec_id);
	if (!acodec){
		cout << "can't find the codec id " << ic->streams[audioStream]->codecpar->codec_id;
		getchar(); return -1;
	}
	cout << "find the AVCodec " << ic->streams[audioStream]->codecpar->codec_id << endl;
	///����������������
	AVCodecContext *ac = avcodec_alloc_context3(acodec);

	///���ý����������Ĳ���
	avcodec_parameters_to_context(ac, ic->streams[audioStream]->codecpar);
	ac->thread_count = 8;	//���߳̽���

	//�򿪽�����������
	re = avcodec_open2(ac, 0, 0);
	if (re != 0){
		char buf[1024] = { 0 };
		av_strerror(re, buf, sizeof(buf)-1);
		cout << "avcodec_open2  failed! :" << buf << endl;
		getchar(); return -1;
	}
	cout << "audio avcodec_open2 success!" << endl;
	

	//--------------�������������----------------------------------------
	///ic->streams[videoStream]
	//malloc AVPacket����ʼ��
	AVPacket *pkt = av_packet_alloc();
	AVFrame *frame = av_frame_alloc();
	
	SwsContext *vctx = NULL;
	unsigned char *rgb = NULL;

	for (;;)
	{
		int re = av_read_frame(ic, pkt);	//��ȡstream�е�һ��frame,������Ƶ����Ƶ

		if (pkt->stream_index != videoStream)	continue;	
		AVFrame*  pFrame = av_frame_alloc();		
		int getPicture = 0;
		
		re = avcodec_decode_video2(vc, pFrame, &getPicture, pkt);
		if (re < 0)
			break;
		if (!getPicture)
			break;



		if (re != 0){
			//ѭ������
			cout << "=================end=================" << endl;
			int ms = 3000; //����λ�� ����ʱ�������������ת��
			long long pos = (double)ms / (double)1000 * r2d(ic->streams[pkt->stream_index]->time_base);
			av_seek_frame(ic, videoStream, pos, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
			continue;
		}
		cout << "pkt->size = " << pkt->size << endl;	//��ʾ��ʱ��
		cout << "pkt->pts = " << pkt->pts << endl;

		//ת��Ϊ���룬������ͬ��
		cout << "pkt->pts ms = " << pkt->pts * (r2d(ic->streams[pkt->stream_index]->time_base) * 1000) << endl;
		cout << "pkt->dts = " << pkt->dts << endl;		//����ʱ��

		AVCodecContext *cc = 0;
		if (pkt->stream_index == videoStream){
			cout << "ͼ��" << endl; cc = vc;	//vc�������
		}
		if (pkt->stream_index == audioStream){
			cout << "��Ƶ" << endl; cc = ac;
		}

		//-----------------������Ƶ---------------
		//����packet�������߳�  send��NULL����ö��receiveȡ�����л���֡
		re = avcodec_send_packet(cc, pkt);		//��������Ƶ����Ƶ
		av_packet_unref(pkt);					//�ͷţ����ü���-1 Ϊ0�ͷſռ䣬send֮��pkt��û����

		if (re != 0){
			char buf[1024] = { 0 };
			av_strerror(re, buf, sizeof(buf)-1);
			cout << "avcodec_send_packet  failed! :" << buf << endl;
			continue;
		}
		
		for (;;)
		{
			//���߳��л�ȡ����ӿ�,һ��send���ܶ�Ӧ���receive
			re = avcodec_receive_frame(cc, frame);			//AVFrame *frame = av_frame_alloc()����ȡ������	
			if (re != 0) break;
			
			if (cc == vc)
			{
				vctx = sws_getCachedContext(
					vctx,	//��NULL���´���
					frame->width, frame->height,		//����Ŀ��
					(AVPixelFormat)frame->format,	//�����ʽ YUV420p
					frame->width, frame->height,	//����Ŀ��
					AV_PIX_FMT_RGBA,				//�����ʽRGBA
					SWS_BILINEAR,					//�ߴ�仯���㷨
					0, 0, 0);
				//if(vctx)
				//cout << "���ظ�ʽ�ߴ�ת�������Ĵ������߻�ȡ�ɹ���" << endl;
				//else
				//	cout << "���ظ�ʽ�ߴ�ת�������Ĵ������߻�ȡʧ�ܣ�" << endl;
				if (vctx)
				{
					if (!rgb) rgb = new unsigned char[frame->width*frame->height * 4];
					uint8_t *data[2] = { 0 };
					data[0] = rgb;
					int lines[2] = { 0 };
					lines[0] = frame->width * 4;

					re = sws_scale(vctx,
						frame->data,		//��������
						frame->linesize,	//�����д�С
						0,
						frame->height,		//����߶�
						data,				//������ݺʹ�С
						lines
						);
					cout << "sws_scale = " << re << endl;
				}
			}
		}	//һ��receive�������

		//XSleep(500);
	}	//for (;;)

	av_frame_free(&frame);
	av_packet_free(&pkt);

	if (ic){
		//�ͷŷ�װ�����ģ����Ұ�ic��0
		avformat_close_input(&ic);
	}

	getchar();
	return 0;
}
*/


/*

#include <iostream>
#include<fstream>
#include <thread>
extern "C"{
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include<libswscale/swscale.h>
#include <libavutil/imgutils.h>
}
using namespace std;
#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"swscale.lib")
#pragma comment(lib, "avutil.lib")

static double r2d(AVRational r)
{
	return r.den == 0 ? 0 : (double)r.num / (double)r.den;
}
void XSleep(int ms)
{
	//c++ 11
	chrono::milliseconds du(ms);
	this_thread::sleep_for(du);
}
int main(int argc, char *argv[]){
	cout << "Test Demux FFmpeg.club" << endl;
	const char *path = "czl.mp4";

	av_register_all();		//��ʼ����װ��

	//��ʼ������� �����Դ�rtsp rtmp http Э�����ý����Ƶ��
	avformat_network_init();
	avcodec_register_all();			//ע�������
	AVDictionary *opts = NULL;		//��������
	//����rtsp����tcpЭ���
	av_dict_set(&opts, "rtsp_transport", "tcp", 0);

	//������ʱʱ��
	av_dict_set(&opts, "max_delay", "500", 0);

	//-----------���ý���--------------------------------
	//���װ������
	AVFormatContext *ic = NULL;
	int re = avformat_open_input(
		&ic,
		path,
		0,		// 0��ʾ�Զ�ѡ������
		&opts	//�������ã�����rtsp����ʱʱ��
		);
	if (re != 0){
		char buf[1024] = { 0 };		//��Ŵ�����Ϣ
		av_strerror(re, buf, sizeof(buf)-1);
		cout << "open " << path << " failed! :" << buf << endl;
		getchar(); return -1;
	}
	cout << "open " << path << " success! " << endl;

	//--------------------------------------------------	
	re = avformat_find_stream_info(ic, 0);				//��ȡ����Ϣ 

	int totalMs = ic->duration / (AV_TIME_BASE / 1000);//��ʱ�� ����
	cout << "totalMs = " << totalMs << endl;

	av_dump_format(ic, 0, path, 0);				//��ӡ��Ƶ����ϸ��Ϣ

	int videoStream = -1; int audioStream = -1;

	//��ȡ����Ƶ����Ϣ(����1������������ȡ)������stream��ȡ����Ƶ��Ϣ
	for (int i = 0; i < ic->nb_streams; i++)
	{
		AVStream *as = ic->streams[i];		//����Ƶ������Ƶ��
		cout << "codec_id = " << as->codecpar->codec_id << endl;
		cout << "format = " << as->codecpar->format << endl;

		if (as->codecpar->codec_type == AVMEDIA_TYPE_AUDIO){
			audioStream = i;
			cout << i << "��Ƶ��Ϣ" << endl;
			cout << "sample_rate = " << as->codecpar->sample_rate << endl;
			//AVSampleFormat;
			cout << "channels = " << as->codecpar->channels << endl;
			//һ֡���ݣ��� ��ͨ�������� 
			cout << "frame_size = " << as->codecpar->frame_size << endl;
			//1024 * 2 * 2 = 4096  fps = sample_rate/frame_size			
		}

		else if (as->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
			videoStream = i;
			cout << i << "��Ƶ��Ϣ" << endl;
			cout << "width=" << as->codecpar->width << endl;
			cout << "height=" << as->codecpar->height << endl;
			//֡�� fps ����ת��
			cout << "video fps = " << r2d(as->avg_frame_rate) << endl;
		}
	}

	//---��ȡ��Ƶ����Ϣ������2��
	videoStream = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);

	//--------------------------------------------------------
	//�ҵ���Ƶ����������Ƶ��������	
	AVCodec *vcodec = avcodec_find_decoder(ic->streams[videoStream]->codecpar->codec_id);
	if (!vcodec){
		cout << "can't find the codec id " << ic->streams[videoStream]->codecpar->codec_id;
		getchar(); return -1;
	}
	cout << "find the AVCodec " << ic->streams[videoStream]->codecpar->codec_id << endl;

	AVCodecContext *vc = avcodec_alloc_context3(vcodec);	//vcodec�����������Ϊ����������vc����ռ�

	///���ý����������Ĳ���
	avcodec_parameters_to_context(vc, ic->streams[videoStream]->codecpar);
	vc->thread_count = 8;		//���߳̽���

	///�򿪽�����������
	re = avcodec_open2(vc, 0, 0);
	if (re != 0){
		char buf[1024] = { 0 };
		av_strerror(re, buf, sizeof(buf)-1);
		cout << "avcodec_open2  failed! :" << buf << endl; getchar(); return -1;
	}
	cout << "video avcodec_open2 success!" << endl;


	//--��Ƶ��������
	//AVCodec *vcodec = avcodec_find_decoder(ic->streams[videoStream]->codecpar->codec_id);//��Ƶ����
	AVCodec *acodec = avcodec_find_decoder(ic->streams[audioStream]->codecpar->codec_id);
	if (!acodec){
		cout << "can't find the codec id " << ic->streams[audioStream]->codecpar->codec_id;
		getchar(); return -1;
	}
	cout << "find the AVCodec " << ic->streams[audioStream]->codecpar->codec_id << endl;
	///����������������
	AVCodecContext *ac = avcodec_alloc_context3(acodec);

	///���ý����������Ĳ���
	avcodec_parameters_to_context(ac, ic->streams[audioStream]->codecpar);
	ac->thread_count = 8;	//���߳̽���

	///�򿪽�����������
	re = avcodec_open2(ac, 0, 0);
	if (re != 0){
		char buf[1024] = { 0 };
		av_strerror(re, buf, sizeof(buf)-1);
		cout << "avcodec_open2  failed! :" << buf << endl;
		getchar(); return -1;
	}
	cout << "audio avcodec_open2 success!" << endl;

	//--------------�������������----------------------------------------
	///ic->streams[videoStream]
	//malloc AVPacket����ʼ��
	AVPacket *pkt  = av_packet_alloc();
	AVFrame *frame = av_frame_alloc();
	ofstream ofile("yuvfile.yuv", ios::binary);
	if (!ofile){
		cerr << "open out file of yuv error\n";
		abort();
	}


	//���ø�ʽ
	AVFrame *pFrame, *pFrameYUV;
	pFrameYUV = av_frame_alloc();
	uint8_t *out_buffer;
	AVCodecContext*  pCodecCtx = ic->streams[videoStream]->codec;
	//	out_buffer = new uint8_t[avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height)];
	out_buffer = (unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1));

	//	avpicture_fill((AVPicture *)pFrameYUV, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);
	av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);

	//--ת��AV_PIX_FMT_YUV420P
	struct SwsContext *img_convert_ctx = NULL;
	img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
	
	//---��ȡframe-----
	while (1)
	{
		int re = av_read_frame(ic, pkt);	//��ȡstream�е�һ��frame,������Ƶ����Ƶ
		if (re != 0){
			//ѭ������			
			int ms = 3000; //����λ�� ����ʱ�������������ת��
			long long pos = (double)ms / (double)1000 * r2d(ic->streams[pkt->stream_index]->time_base);
			av_seek_frame(ic, videoStream, pos, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
			//continue;
			break;
		}
		cout << "pkt->size = " << pkt->size << endl;	//��ʾ��ʱ��
		cout << "pkt->pts = " << pkt->pts << endl;

		//ת��Ϊ���룬������ͬ��
		cout << "pkt->pts ms = " << pkt->pts * (r2d(ic->streams[pkt->stream_index]->time_base) * 1000) << endl;
		cout << "pkt->dts = " << pkt->dts << endl;		//����ʱ��

		AVCodecContext *cc = 0;
		if (pkt->stream_index == videoStream){
			cout << "ͼ��" << endl; cc = vc;	//vc�������
		}
		if (pkt->stream_index == audioStream){
			cout << "��Ƶ" << endl; cc = ac;
			continue;	//������Ƶ����
		}

		//-----------------������Ƶ---------------
		//����packet�������߳�  send��NULL����ö��receiveȡ�����л���֡
		/*
		re = avcodec_send_packet(cc, pkt);			//��������Ƶ����Ƶ
		av_packet_unref(pkt);						//�ͷţ����ü���-1 Ϊ0�ͷſռ䣬send֮��pkt��û����
		if (re != 0){
			char buf[1024] = { 0 };
			av_strerror(re, buf, sizeof(buf)-1);
			cout << "avcodec_send_packet  failed! :" << buf << endl;
			continue;
		}
		for (;;){
			//���߳��л�ȡ����ӿ�,һ��send���ܶ�Ӧ���receive
			re = avcodec_receive_frame(cc, frame);			//AVFrame *frame = av_frame_alloc()����ȡ������	
			if (re != 0) break;
			cout << "recv frame " << frame->format << " " << frame->linesize[0] << endl;
		}	///

		if (pkt->stream_index != videoStream)	continue;
		pFrame = av_frame_alloc();
		int getPicture = 0;
		re = avcodec_decode_video2(vc, pFrame, &getPicture, pkt);
		if (re < 0)
			break;
		if (!getPicture)
			continue;

		//-��������frame��YUV240�ĸ�ʽд���ļ�		
		sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);

		//дYUV
		//fwrite(pFrameYUV->data[0], (pCodecCtx->width)*(pCodecCtx->height), 1, output);
		//fwrite(pFrameYUV->data[1], (pCodecCtx->width)*(pCodecCtx->height) / 4, 1, output);
		//fwrite(pFrameYUV->data[2], (pCodecCtx->width)*(pCodecCtx->height) / 4, 1, output);

		ofile.write((char*)pFrameYUV->data[0], (pCodecCtx->width)*(pCodecCtx->height));
		ofile.write((char*)pFrameYUV->data[1], (pCodecCtx->width)*(pCodecCtx->height) / 4);
		ofile.write((char*)pFrameYUV->data[2], (pCodecCtx->width)*(pCodecCtx->height) / 4);
		av_packet_free(&pkt);
		//дRGB  
		//fwrite(pFrameYUV->data[0], (pCodecCtx->width)*(pCodecCtx->height) * 3, 1, output);
		//XSleep(500);

	}	

	while (1)
	{
		if (pkt->stream_index != videoStream)	continue;
		pFrame = av_frame_alloc();
		int getPicture = 0;
		re = avcodec_decode_video2(vc, pFrame, &getPicture, pkt);
		if (re < 0)
			break;
		if (!getPicture)
			break;
		sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);
		
		ofile.write((char*)pFrameYUV->data[0], (pCodecCtx->width)*(pCodecCtx->height));
		ofile.write((char*)pFrameYUV->data[1], (pCodecCtx->width)*(pCodecCtx->height) / 4);
		ofile.write((char*)pFrameYUV->data[2], (pCodecCtx->width)*(pCodecCtx->height) / 4);
		//av_packet_free(&pkt);
	}

	av_frame_free(&frame);
	av_packet_free(&pkt);
	ofile.close();
	if (ic){
		//�ͷŷ�װ�����ģ����Ұ�ic��0
		avformat_close_input(&ic);
	}

	getchar();
	return 0;
}

*/