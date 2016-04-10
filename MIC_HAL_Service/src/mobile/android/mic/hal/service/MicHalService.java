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

	private native boolean _set(int led);

	static
	{
		System.load("/system/lib/mic_hal_jni.so");
	}
}

