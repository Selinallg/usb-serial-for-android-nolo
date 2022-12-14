package com.module.usbserialforandroid;


import usbserial.driver.CdcAcmSerialDriver;
import usbserial.driver.ProbeTable;
import usbserial.driver.UsbSerialProber;

/**
 * add devices here, that are not known to DefaultProber
 *
 * if the App should auto start for these devices, also
 * add IDs to app/src/main/res/xml/device_filter.xml
 */
class CustomProber {

    static UsbSerialProber getCustomProber() {
        ProbeTable customTable = new ProbeTable();
        //customTable.addProduct(0x16d0, 0x087e, CdcAcmSerialDriver.class); // e.g. Digispark CDC

        customTable.addProduct(0x1915, 0x520F, CdcAcmSerialDriver.class); // e.g. Digispark CDC
        customTable.addProduct(0x248A, 0x880C, CdcAcmSerialDriver.class); // e.g. Digispark CDC
        customTable.addProduct(0x880C, 0x248A, CdcAcmSerialDriver.class); // e.g. Digispark CDC
        customTable.addProduct(9354, 34828 , CdcAcmSerialDriver.class); // e.g. Digispark CDC
        customTable.addProduct(34828, 9354 , CdcAcmSerialDriver.class); // e.g. Digispark CDC
        return new UsbSerialProber(customTable);
    }

}
