#ifndef __BASE_BUFFER_H__
#define __BASE_BUFFER_H__


#include <stdint.h>


namespace Base {


/// �ɶ�̬���������ݻ�����, ֧��С�ڴ��Ż�
class CBuffer
{
	/// ��ֹ���ƹ���͸�ֵ����
	CBuffer(CBuffer const&);
	CBuffer& operator=(CBuffer const&);

public:
	/// ���캯��
	/// \param [in] increase ����ÿ�ζ�̬�����Ĵ�С
	CBuffer(int increase = 64);

	/// ��������
	~CBuffer();

	/// Ԥ���仺������, ���ԭ������������, ������ڴ����·����Լ��ڴ渴�Ʋ���
	/// \param capacity �µĻ�������
	void Reserve(int capacity);

	/// ������Ч���ݳ���, ���ԭ������������, ������ڴ����·����Լ��ڴ渴�Ʋ���
	/// \param size �µ���Ч���ݳ���
    void Resize(int bytes);

	/// ��β��׷������, ���ԭ������������, ������ڴ����·����Լ��ڴ渴�Ʋ���
	/// \param buffer ׷�ӵ�����ָ��
	/// \param length ׷�ӵ����ݳ���
	/// \return ʵ��д�������
    int PutBuffer(void const* buffer, int bytes);

	/// ȡ���ݵ�ַ, PutBuffer Resize Reserve ��������ɵ�ַʧЧ
    void* GetBuffer() const;

	/// ȡ��Ч���ݳ���
    int Size() const;

	/// ȡ��������
	int Capacity() const;

	/// �Ƿ�Ϊ��
	bool Empty() const;

	/// �ͷŻ���, ����Ϊ�ն���
	void Clear();

private:
    intptr_t mInternal[8];
};


} // namespace Base


#endif // __BASE_BUFFER_H__

