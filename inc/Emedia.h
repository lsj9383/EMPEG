#ifndef __EMEDIA_H
#define __EMEDIA_H
#include <string>
#include <memory>

enum VideoType{
	NONE,	//��
	H264,
	MPEG4,
	JPEG2000
};

class Emedia{
public:
	virtual ~Emedia(){};
	virtual const std::string& where() = 0;				// ��Ƶ���ڵľ���·��
	virtual int high() = 0;								// ��Ƶ�ļ���
	virtual int width() = 0;							// ��Ƶ�ļ���
	virtual int64_t frames() = 0;						// ��Ƶ֡��
	virtual double fps() = 0;							// ��Ƶ֡��
	virtual VideoType video_type() = 0;					// ��Ƶ��ʽ

	virtual bool demuxer(const std::string& videoPath, const std::string& audioPath)=0;	//��ȡ��������Ƶ�ļ�
	virtual bool xaudio(const std::string& path) = 0;	// ��ȡ��Ƶ�ļ���ָ��·��
	virtual bool xvideo(const std::string& path) = 0;	// ��ȡ��Ƶ�ļ���ָ��·��
	virtual bool xyuv(const std::string& path) = 0;		// ��ȡ��Ƶ�ļ��е�yuv

protected:
	virtual bool __open__()=0;
	
public:
	static std::shared_ptr<Emedia> generate(const std::string& path);
	static bool combine(const std::string& videoPath, const std::string& audioPath, const std::string& mediaPath);
};

#endif