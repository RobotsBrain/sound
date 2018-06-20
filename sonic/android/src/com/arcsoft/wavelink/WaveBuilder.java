
package com.arcsoft.wavelink;

public class WaveBuilder
{
	public WaveBuilder()
	{
        synchronized (this)
        {
			mHandle = nativeCreate();
		}
	}

	public boolean setContent(String ip, int port, String ssid, String password, String mode)
	{
        synchronized (this)
        {
        	if (mHandle != 0)
        	{
				return nativeSetContent(mHandle, ip, port, ssid, password, mode);
			}
		}

		return false;
	}

	public int readWave(byte[] waveBuffer)
	{
        synchronized (this)
        {
        	if (mHandle != 0)
        	{
        		return nativeReadWave(mHandle, waveBuffer);
        	}
    	}

    	return 0;
    }

    public int getDuration()
    {
        synchronized (this)
        {
            if (mHandle != 0)
            {
                return nativeGetDuration(mHandle);
            }
        }

        return 0;
    }

	public void release()
	{
        synchronized (this)
        {
            if (mHandle != 0)
            {
                nativeDestroy(mHandle);
                mHandle = 0;
            }
        }
   	}

    @Override
    protected void finalize() throws Throwable
    {
        release();
        super.finalize();
    }

    private long mHandle = 0;
    private static native long nativeCreate();
    private static native void nativeDestroy(long handle);
    private static native boolean nativeSetContent(long handle, String ip, int port, String ssid, String password, String mode);
    private static native int nativeReadWave(long handle, byte[] waveBuffer);
    private static native int nativeGetDuration(long handle);

	static {
		System.loadLibrary("wavelink");
	}
}

