package com.nextone.util;


/**
 * this class takes in a time in milliseconds and breaks them into
 * days, hours, minutes, seconds and milliseconds
 */

public class DeltaTime {
	  private final int days, hrs, mins, secs, msecs;
	  private final long origTime;

	  public DeltaTime (long time) {
		 origTime = time;
		 msecs = (int)(time%1000);
		 long seconds = time/1000;
		 long minutes = seconds/60;
		 long hours = minutes/60;
		 days = (int)hours/24;
		 secs = (int)( seconds - (minutes*60) );
		 mins = (int)( minutes - (hours*60) );
		 hrs = (int)( hours - (days*24) );
	  }

	  public int getDays () {
		 return days;
	  }

	  public int getHours () {
		 return hrs;
	  }

	  public int getMinutes () {
		 return mins;
	  }

	  public int getSeconds () {
		 return secs;
	  }

	  public int getMilliSeconds () {
		 return msecs;
	  }

	  // returns the original time
	  public long getTime () {
		 return origTime;
	  }

	  public String toString () {
		 StringBuffer sb = new StringBuffer();
		 if (days != 0) {
			sb.append(days);
			if (days == 1)
			   sb.append(" day, ");
			else
			   sb.append(" days, ");
		 }
		 if (hrs != 0) {
			sb.append(hrs);
			if (hrs == 1)
			   sb.append(" hour, ");
			else
			   sb.append(" hours, ");
		 }
		 if (mins != 0) {
			sb.append(mins);
			if (mins == 1)
			   sb.append(" minute, ");
			else
			   sb.append(" minutes, ");
		 }
		 if (secs != 0) {
			sb.append(secs);
			if (secs == 1)
			   sb.append(" second, ");
			else
			   sb.append(" seconds, ");
		 }
		 sb.append(msecs);
		 if (msecs <= 1)
			sb.append(" millisecond");
		 else
			sb.append(" milliseconds");

		 return sb.toString();
	  }

}
