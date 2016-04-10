package com.android.mic.hal.service;

import android.app.Activity;
import android.os.Bundle;

public class MicHalServiceMain extends Activity
{
	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.main);
		MicHalService micHalService = MicHalService.getInstance();
		micHalService.set(0, 0);
	}  
}   
