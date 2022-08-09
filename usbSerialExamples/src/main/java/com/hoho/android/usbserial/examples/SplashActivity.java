package com.hoho.android.usbserial.examples;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.os.Bundle;
import android.util.Log;

public class SplashActivity extends AppCompatActivity {

    private static final String TAG = "SplashActivity";
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Intent intent = getIntent();
        String action = intent.getAction();
        Log.d(TAG, "onCreate: action="+action);
        if (action.equals("android.hardware.usb.action.USB_DEVICE_ATTACHED")){
            Intent mainIntent = new Intent(this,MainActivity.class);
            startActivity(mainIntent);
            Log.d(TAG, "onCreate: sucess");
            finish();
        }
    }

    @Override
    protected void onDestroy() {
        Log.d(TAG, "onDestroy: ");
        super.onDestroy();
    }
}