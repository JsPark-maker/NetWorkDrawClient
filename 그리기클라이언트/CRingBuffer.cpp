#include <iostream>
#include "CRingBuffer.h"

CRingBuffer::CRingBuffer()
	:front(0), rear(0)
{
	bufferPtr = new char[MAX];
	bufSize = MAX;
}

CRingBuffer::CRingBuffer(int iBufferSize)
	:front(0), rear(0)
{
	bufferPtr = new char[iBufferSize];
	bufSize = iBufferSize;
}

void CRingBuffer::Initial(int iBufferSize)
{
}

void CRingBuffer::Resize(int size)
{
}

int CRingBuffer::GetBufferSize()
{
	return bufSize;
}

int CRingBuffer::GetUseSize()
{
	if (rear >= front)
		return rear - front;
	else
		return bufSize - front + rear;
}

int CRingBuffer::GetFreeSize()
{
	return bufSize - GetUseSize();
}

int CRingBuffer::Enqueue(char* chpData, int iSize)
{
	//rear를 옮기는것을 마지막으로 빼라
	char* chpDataBuf = chpData;
	int inputSize;
	for (inputSize = 0; inputSize < iSize; inputSize++)
	{
		if ((rear + 1) % bufSize == front) //큐가 꽉 찼음
			return inputSize;

		//이쪽으로 왔다는것은 큐가 비어있어서 넣기가 가능하다는 의미
		rear = (rear + 1) % bufSize;

		bufferPtr[rear] = *chpDataBuf;

		chpDataBuf++;
	}

	return inputSize;
}

int CRingBuffer::Dequeue(char* chpDest, int iSize)
{
	int dequeueSize;
	for (dequeueSize = 0; dequeueSize < iSize; dequeueSize++)
	{
		if (front == rear)
			return dequeueSize;

		front = (front + 1) % bufSize;

		chpDest[dequeueSize] = bufferPtr[front];
	}
	return dequeueSize;
}

int CRingBuffer::Peek(char* chpDest, int iSize)
{
	int peekSize;
	int frontBuf = front;
	for (peekSize = 0; peekSize < iSize; peekSize++)
	{
		if (frontBuf == rear)
			return peekSize;

		frontBuf = (frontBuf + 1) % bufSize;

		chpDest[peekSize] = bufferPtr[frontBuf];
	}
}

void CRingBuffer::MoveRear(int iSize)
{
	rear = (rear + iSize) % bufSize;
}

int CRingBuffer::MoveFront(int iSize)
{
	front = (front + iSize) % bufSize;
	return 0;
}

void CRingBuffer::ClearBuffer(void)
{
	front = rear = 0;
}

char* CRingBuffer::GetFrontBufferPtr(void)
{
	return &(bufferPtr[front]);
}

char* CRingBuffer::GetRearBufferPtr(void)
{
	return &(bufferPtr[rear]);
}

void CRingBuffer::Display(void)
{
	for (int i = front; front != rear; i = (i + 1) % bufSize)
	{
		printf("%c", bufferPtr[i]);
	}
}
