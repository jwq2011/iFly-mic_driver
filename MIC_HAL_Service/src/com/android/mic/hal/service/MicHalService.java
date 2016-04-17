package com.android.mic.hal.service;
  

public class MicHalService
{
	private static MicHalService micHalService;

	public static MicHalService getInstance()
	{
		if (micHalService == null)
		    micHalService = new MicHalService();
		
		return micHalService;
	}
/*
	private MicHalService()  
	{  
		init(); 
	}
    
	public boolean init()
	{
		return _init();
	} 
	public boolean set(int mic, int value)
	{
		return _set(mic, value);
	}  

	//  native method 
	private native boolean _init();
	private native boolean _set(int mic, int value);
*/
	public static int open(){
		return nativeOpen();
	}
	private static int close(){
		return nativeClose();
	}
	private static int ChangeFuncMode(int mode){
		return nativeChangeFuncMode(mode);
	}
	private static int GetWakeupSign(){
		return nativeGetWakeupSign();
	}
	private static int GetFuncMode(){
		return nativeGetFuncMode();
	}
	private static int GetEnvironmentMode(){
		return nativeGetEnvironmentMode();
	}
	private static int ChangeEnvironmentMode(int mode){
		return nativeChangeEnvironmentMode();
	}


	private native static int nativeOpen();
	private native static int nativeClose();
	private native static int nativeChangeFuncMode(int mode);
	private native static int nativeGetWakeupSign();
	private native static int nativeGetFuncMode();
	private native static int nativeGetEnvironmentMode();
	private native static int nativeChangeEnvironmentMode(int mode);

	static
	{
		System.load("/system/lib/mic_hal_jni.so");
	}
}

