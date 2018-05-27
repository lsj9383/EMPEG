#ifndef __EMEDIA_IMPL_H
#define __EMEDIA_IMPL_H

#include "Emedia.h"
#include<hash_map>

#define USE_H264BSF 1			//'1': Use H.264 Bitstream Filter   
#define USE_AACBSF  0			//'1': Use AAC Bitstream Filter 

class EmediaImpl : public Emedia{
public:
	~EmediaImpl();
	EmediaImpl(const std::string& path);
	const std::string& where() override;	
	int high() override;
	int width() override;
	int64_t frames() override;
	double fps() override ;
	VideoType video_type() override;

	bool demuxer(const std::string& videoPath, const std::string& audioPath);
	bool xaudio (const std::string& path) override;
	bool xvideo (const std::string& path) override;
	bool xyuv   (const std::string& path) override;
	//bool combine(const std::string& videoPath, const std::string& audioPath, const std::string& mediaPath) override;
protected:
	bool _open_() override;
	bool _read_frame(AVPacket& pkt);
	bool _decode    (AVPacket* pkt, AVFrame& yuv);

	void func1();
	//bool _yuv_rgb();
private:
	std::string _filePath;                            //Դ�ļ�

	AVFormatContext* _formatCtx = NULL;               //Դ�ļ���ʽ
	AVFormatContext* _ofmt_ctx_v = NULL;              //�����Ƶ��ʽ
	AVOutputFormat*  _ofmt_v = NULL;
	int _videoStream; 
	int _audioStream;								 //����Ƶ��������ȡʱ��������Ƶ
	int _flag;
	int _ret = 0;
    //hash_map<AVCodecID, VideoType> _videoTypeMap;
};

#endif