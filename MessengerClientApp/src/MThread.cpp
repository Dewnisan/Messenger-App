/*
 * Mthread.cpp
 */

#include "MThread.h"
#include <iostream>
#include <stdio.h>
using namespace std;

void* worker(void* arg){
	MThread* threadObj = (MThread*)arg;
	threadObj->run();
	threadObj->threadId = 0;
	return NULL;
}

void MThread::start(){
	pthread_create(&threadId,NULL,worker,(void*)this);
}

void MThread::waitForThread(){
	// Prevent bug
	if (threadId>0){
		pthread_join(threadId,NULL);
		threadId=0;
	}
}

MThread::~MThread(){
	if (threadId>0){
		//kills the thread if exist
		pthread_cancel(threadId);
		printf("Thread was canceled\n");
	}
}

