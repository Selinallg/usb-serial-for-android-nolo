package com.module.usbserialforandroid;

import android.app.Activity;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.Log;

import java.io.IOException;
import java.util.Arrays;
import java.util.EnumSet;

import usbserial.driver.UsbSerialDriver;
import usbserial.driver.UsbSerialPort;
import usbserial.driver.UsbSerialProber;
import usbserial.util.SerialInputOutputManager;

public class TerminalClient implements SerialInputOutputManager.Listener {

    // Used to load the 'nativedemo' library on application startup.
    static {
        System.loadLibrary("usbtoserial");
    }

    private static final String TAG = "TerminalClient";
    private enum UsbPermission {Unknown, Requested, Granted, Denied}

    private static final String INTENT_ACTION_GRANT_USB = BuildConfig.LIBRARY_PACKAGE_NAME + ".GRANT_USB";
    private static final int WRITE_WAIT_MILLIS = 2000;
    private static final int READ_WAIT_MILLIS = 2000;

    private int deviceId, portNum, baudRate;
    private boolean withIoManager = true;

    private final Handler mainLooper;
    private final HandlerThread handlerThread;

    private SerialInputOutputManager usbIoManager;
    private UsbSerialPort usbSerialPort;
    private UsbPermission usbPermission = UsbPermission.Unknown;
    private boolean connected = false;
    private Activity mActivity;
    private UsbDataListener usbDataListener = null;

    public TerminalClient(Activity activity, int deviceId, int portNum, int baudRate) {
        mActivity = activity;
        this.deviceId = deviceId;
        this.portNum = portNum;
        this.baudRate = baudRate;

        handlerThread = new HandlerThread("TerminalClient");
        handlerThread.start();
        mainLooper = new Handler(handlerThread.getLooper());
    }

    public interface UsbDataListener {
        public void imuData(long timestamp, short acc_X, short acc_Y, short acc_Z, short gyro_X, short gyro_Y, short gyro_Z);

        public void nsyncData(long timestamp);

        public void receive(byte[] data);
    }

    /*
     * Serial
     */
    @Override
    public void onNewData(byte[] data) {
        mainLooper.post(() -> {
            receive(data);
        });
    }

    @Override
    public void onRunError(Exception e) {
        mainLooper.post(() -> {
            disconnect();
        });
    }

    /*
     * Serial + UI
     */
    public boolean connect() {
        if (mActivity == null) {
            return false;
        }
        if (connected) {
            return true;
        }
        UsbManager usbManager = (UsbManager) mActivity.getSystemService(Context.USB_SERVICE);
        if (usbManager == null) {
            return false;
        }
        UsbDevice device = null;
        for (UsbDevice v : usbManager.getDeviceList().values()) {
            Log.d(TAG, "connect: "+v.getProductId() +"| "+v.getVendorId());
            Log.d(TAG, "connect: deviceId="+deviceId);
            if (v.getDeviceId() == deviceId) {
                device = v;
            }
        }
        if (device == null) {
            return false;
        }
        UsbSerialDriver driver = UsbSerialProber.getDefaultProber().probeDevice(device);
        if (driver == null) {
            driver = CustomProber.getCustomProber().probeDevice(device);
        }
        if (driver == null) {
            return false;
        }
        if (driver.getPorts().size() < portNum) {
            return false;
        }
        usbSerialPort = driver.getPorts().get(portNum);
        UsbDeviceConnection usbConnection = usbManager.openDevice(driver.getDevice());

        if (usbConnection == null && usbPermission == UsbPermission.Unknown && !usbManager.hasPermission(driver.getDevice())) {
            usbPermission = UsbPermission.Requested;
            PendingIntent usbPermissionIntent = PendingIntent.getBroadcast(mActivity, 0, new Intent(INTENT_ACTION_GRANT_USB), 0);
            usbManager.requestPermission(driver.getDevice(), usbPermissionIntent);
            return false;
        }
        if (usbConnection == null) {
            if (!usbManager.hasPermission(driver.getDevice()))
                Log.e("connect", "connection failed: permission denied");
            else
                Log.e("connect", "connection failed: open failed");
            return false;
        }

        try {
            usbSerialPort.open(usbConnection);
            //      usbSerialPort.setParameters(baudRate, 8, 1, UsbSerialPort.PARITY_NONE);
            if (withIoManager) {
                usbIoManager = new SerialInputOutputManager(usbSerialPort, this);
                usbIoManager.start();
            }
            connected = true;
        } catch (Exception e) {
            disconnect();
        }

        return connected;
    }

    public void disconnect() {
        connected = false;
        if (usbIoManager != null) {
            usbIoManager.setListener(null);
            usbIoManager.stop();
        }
        usbIoManager = null;
        try {
            usbSerialPort.close();
        } catch (IOException ignored) {
        }
        usbSerialPort = null;
    }

    public void send(byte[] data) {
        if (!connected) {
            return;
        }
        try {
            byte[] src = {0x02, 0x00, 0x01, 0x01};
            usbSerialPort.write(data, WRITE_WAIT_MILLIS);
        } catch (Exception e) {
            onRunError(e);
        }
    }

    public void read() {
        if (!connected) {
            return;
        }
        try {
            byte[] buffer = new byte[8192];
            int len = usbSerialPort.read(buffer, READ_WAIT_MILLIS);
            receive(Arrays.copyOf(buffer, len));
        } catch (IOException e) {
            disconnect();
        }
    }

    public boolean isConnected() {
        return connected;
    }

    public void setUsbDataCallBack(UsbDataListener lister) {
        usbDataListener = lister;
    }

    public EnumSet<UsbSerialPort.ControlLine> getSupportedControlLines() throws IOException {
        if (usbSerialPort != null) {
            return usbSerialPort.getSupportedControlLines();
        }
        return null;
    }

    public EnumSet<UsbSerialPort.ControlLine> getControlLines() throws IOException {
        if (usbSerialPort != null) {
            return usbSerialPort.getControlLines();
        }
        return null;
    }

    public void setRTS(boolean rts) throws IOException {
        usbSerialPort.setRTS(rts);
    }

    public void setDTR(boolean dtr) throws IOException {
        usbSerialPort.setDTR(dtr);
    }

    private void receive(byte[] data) {
        if (data.length > 0) {
            updateDatas(data);
            if (usbDataListener != null) {
                usbDataListener.receive(data);
            }
        }
    }

    public void imuData(long timestamp, short acc_X, short acc_Y, short acc_Z, short gyro_X, short gyro_Y, short gyro_Z) {
        if (usbDataListener != null) {
            usbDataListener.imuData(timestamp, acc_X, acc_Y, acc_Z, gyro_X, gyro_Y, gyro_Z);
        }
    }

    public void nsyncData(long timestamp) {
        // TODO: 2022/8/10 modify by llg
        if (usbDataListener != null) {
            usbDataListener.nsyncData(timestamp);
        }
    }

    public native void updateDatas(byte[] datas);
}
