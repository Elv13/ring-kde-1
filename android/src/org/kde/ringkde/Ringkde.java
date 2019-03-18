package org.kde.ringkde;

import org.qtproject.qt5.android.bindings.QtActivity;
import org.qtproject.qt5.android.bindings.QtApplication;
import android.util.Log;
import android.content.Context;
import android.os.Bundle;
import android.media.AudioAttributes;
import java.util.Locale;
import java.lang.String;

public class Ringkde extends QtActivity
{
    public Ringkde()
    {
    }

    static public void foo() {
        System.out.println(">>>>>>>>>>>>THIS IS CALLED foo");
    }

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);
       System.out.println(">>>>>>>>>>>> ACTIVITY foo");
    }

    // Until services are imlemented (if ever), quit when paused, it just
    // wont resume properly.

    @Override
    public void onPause() {
        System.out.println(">>>>>>>>>>>> ON PAUSE");
        super.onPause(); 
    }

    public void onInit(int status)
    {
    }

    @Override 
    public void onStop() {
        System.out.println(">>>>>>>>>>>> ON STOP");
        super.onStop(); 
    }

    @Override
    public void onResume() {
        System.out.println(">>>>>>>>> ON RESUME");
        super.onResume();
    }


    @Override
    protected void onDestroy() {
            super.onDestroy();
            /*if (m_tts != null) {
                m_tts.shutdown();
            }*/
    }

}

