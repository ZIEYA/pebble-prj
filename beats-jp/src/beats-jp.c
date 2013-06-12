#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include <math.h>


#define MY_UUID { 0x93, 0x7D, 0xD8, 0x33, 0xCA, 0xDE, 0x4B, 0x13, 0xAD, 0x73, 0x05, 0xF5, 0xDE, 0x90, 0xFA, 0x2A }
PBL_APP_INFO( MY_UUID,  "beats-jp",  "zieya-labo",  1,  0, DEFAULT_MENU_ICON,  APP_INFO_WATCH_FACE);

Window		window;
TextLayer	gmtHeader;
TextLayer	gmtTime;
TextLayer	localHeader;
TextLayer	localTime;
TextLayer	internetTimeHeader;
TextLayer	internetTime;
bool timerHasStarted = false;
PblTm	initialinternetTime;




static char* _utoa(unsigned int value, char *s, int radix) 
{
	char*	s1 = s;
	char*	s2 = s;
	
	do {
		*s2++ = "0123456789abcdefghijklmnopqrstuvwxyz"[value % radix];
		value /= radix;
	} while (value > 0);
	
	*s2-- = '\0';
	
	while (s1 < s2) {
		char c = *s1;
		*s1++ = *s2;
		*s2-- = c;
	}
	return s;
}

static char* _itoa(int value, char* s, int radix)
{
	unsigned int t = value;
	char* ss = s;
	
	if (value < 0 && radix == 10) {
		*ss++ = '-';
		t = -t;
	}
	_utoa(t, ss, radix);
	return s;
}

static char* format_internettime_string( double dblBeats, char* strBeats )
{
	double dblIntegers = 0L;
	double dblDecimals =0L;
	char wk[16];
	strcpy( wk, "" );
	
	dblDecimals = modf( dblBeats, &dblIntegers );
	
	if ( (int)dblIntegers < 10 ) {
		strcpy( strBeats, "@00" );
	}
	else {
		if ( (int)dblIntegers < 100 ) {
			strcpy( strBeats, "@0" );
		}
		else {
			strcpy( strBeats, "@" );
		}
	}
	
	_itoa( (int)dblIntegers, wk, 10 );
	strcat( strBeats, wk );
	strcat( strBeats, "." );
	
	dblDecimals = dblDecimals * 100L;
	
	strcpy( wk, "" );
	_itoa( (int)dblDecimals, wk, 10 );
	if ( (int)dblDecimals < 10 ) {
		strcat( strBeats, "0" );
	}
	strcat( strBeats, wk );
	return strBeats;
	
}


void handle_tick(AppContextRef ctx, PebbleTickEvent *t)
{
	static char localHeaderText[] = "JST";
	static char localTimeText[] = "00:00:00";
	static char gmtHeaderText[] = "GMT";
	static char gmtTimeText[] = "00:00:00";
	static char internetTimeHeaderText[] = "Swatch .beat";
	static char internetTimeText[] = "@000.00";
	
	
	int GMTOffset = 9;		//GMT : JST-9
	
	//Set the header text for static headers.
	text_layer_set_text(&localHeader, localHeaderText);
	text_layer_set_text(&gmtHeader, gmtHeaderText);
	text_layer_set_text(&internetTimeHeader, internetTimeHeaderText);
	
	(void) t;
	(void)ctx;
	
	// JST 2 GMT
	PblTm	tGmtTime;
	memcpy( &tGmtTime, t->tick_time, sizeof(tGmtTime));
	
	int convertingHour = t->tick_time->tm_hour - GMTOffset;
	if (convertingHour < 0) {
		convertingHour = 24 + convertingHour;
	}
	else if (convertingHour > 24) {
		convertingHour = convertingHour - 24;
	}
	tGmtTime.tm_hour = convertingHour;
	
	// GMT 2  Swatch Beats 
	double dblBeats = 0L;
	dblBeats = (double)(((tGmtTime.tm_sec + (tGmtTime.tm_min*60) + ( tGmtTime.tm_hour * 3600)  + 3600 ) * 1000)) / 86400L;
	
	if ( dblBeats > 1000L ) {
		dblBeats = dblBeats - 1000L;
	}
	if ( dblBeats < 0L ) {
		dblBeats = dblBeats + 1000L;
	}
	
	
	string_format_time(gmtTimeText, sizeof(gmtTimeText), "%T", &tGmtTime);
	text_layer_set_text(&gmtTime, gmtTimeText);

	//JST
	string_format_time(localTimeText, sizeof(localTimeText), "%T", t->tick_time);
	text_layer_set_text(&localTime, localTimeText);
	
	
	//Swatch .beat
	//	sprintf( internetTimeText, "@-%03.02F", dblBeats );
	
	text_layer_set_text(&internetTime, format_internettime_string( dblBeats, internetTimeText ) );
	
}


void handle_init(AppContextRef ctx) {
	(void)ctx;
	window_init(&window, "Beats@jp");
	window_stack_push(&window, true /* Animated */);
	window_set_background_color(&window, GColorBlack);
	
	//Local header
	text_layer_init(&localHeader, window.layer.frame);
	layer_set_frame(&localHeader.layer, GRect( 16, 10, 128, 25));
	text_layer_set_text_color(&localHeader, GColorWhite);
	text_layer_set_background_color(&localHeader, GColorClear);
	text_layer_set_font(&localHeader, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
	layer_add_child(&window.layer, &localHeader.layer);
	
	//Local text
	text_layer_init(&localTime, window.layer.frame);
	layer_set_frame(&localTime.layer, GRect( 16, 25, 128, 35));
	text_layer_set_text_color(&localTime, GColorWhite);
	text_layer_set_background_color(&localTime, GColorClear);
	text_layer_set_font(&localTime, fonts_get_system_font(FONT_KEY_DROID_SERIF_28_BOLD));
	layer_add_child(&window.layer, &localTime.layer);
	
	//gmt header
	text_layer_init(&gmtHeader, window.layer.frame);
	layer_set_frame(&gmtHeader.layer, GRect( 16, 60, 128, 25));
	text_layer_set_text_color(&gmtHeader, GColorWhite);
	text_layer_set_background_color(&gmtHeader, GColorClear);
	text_layer_set_font(&gmtHeader, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
	layer_add_child(&window.layer, &gmtHeader.layer);
	
	//gmt text
	text_layer_init(&gmtTime, window.layer.frame);
	layer_set_frame(&gmtTime.layer, GRect( 16, 75, 128, 35));
	text_layer_set_text_color(&gmtTime, GColorWhite);
	text_layer_set_background_color(&gmtTime, GColorClear);
	text_layer_set_font(&gmtTime, fonts_get_system_font(FONT_KEY_DROID_SERIF_28_BOLD));
	layer_add_child(&window.layer, &gmtTime.layer);
	
	//internetTime header
	text_layer_init(&internetTimeHeader, window.layer.frame);
	layer_set_frame(&internetTimeHeader.layer, GRect( 16, 110, 128, 25));
	text_layer_set_text_color(&internetTimeHeader, GColorWhite);
	text_layer_set_background_color(&internetTimeHeader, GColorClear);
	text_layer_set_font(&internetTimeHeader, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
	layer_add_child(&window.layer, &internetTimeHeader.layer);
	
	//internetTime text
	text_layer_init(&internetTime, window.layer.frame);
	layer_set_frame(&internetTime.layer, GRect( 16, 125, 128, 35));
	text_layer_set_text_color(&internetTime, GColorWhite);
	text_layer_set_background_color(&internetTime, GColorClear);
	text_layer_set_font(&internetTime, fonts_get_system_font(FONT_KEY_DROID_SERIF_28_BOLD));
	layer_add_child(&window.layer, &internetTime.layer);	
}


void pbl_main(void *params) {
	PebbleAppHandlers handlers = {
				.init_handler = &handle_init,
				.tick_info = {
					.tick_handler =	&handle_tick,
					.tick_units =		SECOND_UNIT
				}
			};
	app_event_loop(params, &handlers);
}
