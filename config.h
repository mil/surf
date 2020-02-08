static int surfuseragent    = 1;  /* Append Surf version to default WebKit user agent */
static char *fulluseragent  = ""; /* Or override the whole user agent string */
static char *scriptfile     = "~/.surf/script.js";
static char *styledir       = "~/.surf/styles/";
static char *certdir        = "~/.surf/certificates/";
static char *cachedir       = "/tmp/.surf_cache";
static char *cookiefile     = "/tmp/.surf_cookies";
static char *searchengine   = "https://duckduckgo.com/?q=";
static char *externalpipe_sigusr1[] =    { "/bin/sh", "-c", "externalpipe_buffer.sh surf_strings_read", winid};
static char *editbuffer[] =              { "/bin/sh", "-c", "edit_screen.sh" };
static char *linkselect_curwin [] =      { "/bin/sh", "-c", "surf_linkselect.sh $0 'Link' | xargs -r xprop -id $0 -f _SURF_GO 8s -set _SURF_GO", winid };
static char *linkselect_newwin [] =      { "/bin/sh", "-c", "surf_linkselect.sh $0 'Link (new window)' | xargs -r surf", winid };
static char *linkselect_yank [] =        { "/bin/sh", "-c", "surf_linkselect.sh $0 'Link (yank)' | y", winid };
static char *linkselect_urlhandler[] =   { "/bin/sh", "-c", "surf_linkselect.sh $0 'URL handler' | xargs -r urlhandler", winid };
static char *codeblock_yank[] =          { "/bin/sh", "-c", "surf_tagyank.sh $0 'Codeblock'", winid };
static char *string_yank[] = { "/bin/sh", "-c", "sed -e 's/<[^>]*>//g' | strings_extract.sh | dmenu -p Copy -l 10 -i -w $(xdotool getactivewindow) | tr -d '\n' | y", winid };

static char *piped_find[] = { 
  "/bin/sh", "-c",
  "sed -e 's/<[^>]*>//g' |\
   strings_extract.sh |\
   dmenu -t -p Find -l 10 -i -w $(xdotool getactivewindow) |\
   xargs -IBB -r xprop -id $0 -f _SURF_FIND 8s -set _SURF_FIND 'BB'\
  ", 
  winid
};

static char *piped_open[] = { "/bin/sh", "-c", 
  "STRINGS=$(sed -e 's/<[^>]*>//g' | strings_extract.sh) \
   PARTS=$(urlparts $(xprop $(xdotool getactivewindow) | grep _SURF_URI | grep -oE '".+"'  | tr -d '"')) \
   BANGS=$(~/.ddg_bangs) \
   echo "$PARTS" "$STRINGS" "$BANGS" \
   dmenu -t -p Open -l 10 -i -w $(xdotool getactivewindow) |\
   xargs -IBB -r xprop -id $0 -f _SURF_GO 8s -set _SURF_GO 'BB' \
  ", 
  winid
};

static char *piped_opennew[] = { "/bin/sh", "-c", 
  "sed -e 's/<[^>]*>//g' |\
   strings_extract.sh |\
   cat - ~/.ddg_bangs |\
   dmenu -t -p Open -l 10 -i -w $(xdotool getactivewindow) |\
   xargs -IBB -r surf 'BB'
  ", 
  winid
};

static char *image_select[] =            { "/bin/sh", "-c", "surf_linkselect.sh $0 'Image (new window)' 'img' 'src' 'alt' | xargs -r surf", winid };

/* Webkit default features */
/* Highest priority value will be used.
 * Default parameters are priority 0
 * Per-uri parameters are priority 1
 * Command parameters are priority 2
 */
static Parameter defconfig[ParameterLast] = {
	/* parameter                    Arg value       priority */
	[AcceleratedCanvas]   =       { { .i = 1 },     },
	[AccessMicrophone]    =       { { .i = 0 },     },
	[AccessWebcam]        =       { { .i = 0 },     },
	[Certificate]         =       { { .i = 0 },     },
	[CaretBrowsing]       =       { { .i = 1 },     },
	[CookiePolicies]      =       { { .v = "aA@" }, },
	[DefaultCharset]      =       { { .v = "UTF-8" }, },
	[DiskCache]           =       { { .i = 1 },     },
	[DNSPrefetch]         =       { { .i = 0 },     },
	[FileURLsCrossAccess] =       { { .i = 0 },     },
	[FontSize]            =       { { .i = 12 },    },
	[FrameFlattening]     =       { { .i = 0 },     },
	[Geolocation]         =       { { .i = 0 },     },
	[HideBackground]      =       { { .i = 0 },     },
	[Inspector]           =       { { .i = 1 },     },
	[Java]                =       { { .i = 1 },     },
	[JavaScript]          =       { { .i = 0 },     },
	[KioskMode]           =       { { .i = 0 },     },
	[LoadImages]          =       { { .i = 1 },     },
	[MediaManualPlay]     =       { { .i = 1 },     },
	[Plugins]             =       { { .i = 1 },     },
	[PreferredLanguages]  =       { { .v = (char *[]){ NULL } }, },
	[RunInFullscreen]     =       { { .i = 0 },     },
	[ScrollBars]          =       { { .i = 1 },     },
	[ShowIndicators]      =       { { .i = 1 },     },
	[SiteQuirks]          =       { { .i = 1 },     },
	[SmoothScrolling]     =       { { .i = 0 },     },
	[SpellChecking]       =       { { .i = 0 },     },
	[SpellLanguages]      =       { { .v = ((char *[]){ "en_US", NULL }) }, },
	[StrictTLS]           =       { { .i = 1 },     },
	[Style]               =       { { .i = 1 },     },
	[WebGL]               =       { { .i = 1 },     },
	[ZoomLevel]           =       { { .f = 1.0 },   },
};

 static UriParameters uriparams[] = {
 	{ "(://|\\.)suckless\\.org(/|$)", {
 	  [JavaScript] = { { .i = 0 }, 1 },
 	  [Plugins]    = { { .i = 0 }, 1 },
 	}, },
 	{ "https?://clojuredocs.org(/|$)", {
 	  [JavaScript] = { { .i = 1 }, 1 },
 	}, },
 	{ "https?://github.com(/|$)", {
 	  [JavaScript] = { { .i = 1 }, 1 },
 	}, },
 	{ "https?://news.ycombinator.com.com(/|$)", {
 	  [ZoomLevel] = { { .i = 1 }, 1 },
 	}, },
 };
;

/* default window size: width, height */
static int winsize[] = { 800, 600 };
static WebKitFindOptions findopts = WEBKIT_FIND_OPTIONS_CASE_INSENSITIVE |
                                    WEBKIT_FIND_OPTIONS_WRAP_AROUND;

#define PROMPT_GO   "URL"
#define PROMPT_GONEW   "URL (new window)"
#define PROMPT_FIND "Find"

/* SETPROP(readprop, setprop, prompt)*/
#define SETPROP(r, s, p) { \
        .v = (const char *[]){ "/bin/sh", "-c", \
             "prop=\"$(printf '%b' \"$(xprop -id $1 $2 " \
             "| sed \"s/^$2(STRING) = //;s/^\\\"\\(.*\\)\\\"$/\\1/\")\" " \
             "| dmenu -h 30 -p \"$4\" -w $1)\" && xprop -id $1 -f $3 8s -set $3 \"$prop\"", \
             "surf-setprop", winid, r, s, p, NULL \
        } \
}
 
/* SETPROP(readprop, setprop, prompt)*/
#define SPAWN_NEW(r, s, p) { \
        .v = (const char *[]){ "/bin/sh", "-c", \
             "prop=\"$(printf '%b' \"$(xprop -id $1 $2 " \
             "| sed \"s/^$2(STRING) = //;s/^\\\"\\(.*\\)\\\"$/\\1/\")\" " \
             "| cat - ~/.ddg_bangs" \
             "| dmenu -l 10 -t -p \"$4\" -w $1)\" && surf \"$prop\"", \
             "surf-setprop", winid, r, s, p, NULL \
        } \
}
/* SETPROP(readprop, setprop, prompt)*/
#define SETPROP_URI(r, s, p) { \
        .v = (const char *[]){ "/bin/sh", "-c", \
             "prop=\"$(printf '%b' \"$(xprop -id $1 $2 " \
             "| sed \"s/^$2(STRING) = //;s/^\\\"\\(.*\\)\\\"$/\\1/\")\" " \
             "| xargs urlparts " \
             "| cat - ~/.ddg_bangs | externalpipe_buffer.sh pipe_combine" \
             "| dmenu -i -t -l 10 -p \"$4\" -w $1)\" && xprop -id $1 -f $3 8s -set $3 \"$prop\"", \
             "surf-setprop", winid, r, s, p, NULL \
        } \
}

#define URLHANDLER { \
	.v = (char *[]){ "/bin/sh", "-c", \
	     "urlhandler $0", winid, NULL \
	} \
}
#define ADDBMK { \
	.v = (char *[]){ "/bin/sh", "-c", \
	     "surf_bookmarkurl $0", winid, NULL \
	} \
}
#define LOADBMK(r, s, p) { \
	.v = (const char *[]){ "/bin/sh", "-c", \
	     "prop=\"$(surf_loadbookmark $1)\" && xprop -id $1 -f $3 8s -set $3 \"$prop\"", \
	     "surf-setprop", winid, r, s, p, NULL \
	} \
}


#define SELNAV { \
  .v = (char *[]){ "/bin/sh", "-c", \
    "prop=\"`xprop -id $0 _SURF_HIST" \
    " | sed -e 's/^.[^\"]*\"//' -e 's/\"$//' -e 's/\\\\\\n/\\n/g'" \
    " | dmenu -p History -w $(xdotool getactivewindow) -i -l 10`\"" \
    " && xprop -id $0 -f _SURF_NAV 8s -set _SURF_NAV \"$prop\"", \
    winid, NULL \
  } \
}


/* DOWNLOAD(URI, referer) */
#define DOWNLOAD(d, r) { \
	.v = (char *[]){ "/bin/sh", "-c", \
		"cd ~/Downloads;"\
		"st -e /bin/sh -c \"echo Downloading via aria2c: $0; aria2c -U '$1'" \
		" --referer '$2' --load-cookies $3 --save-cookies $3 '$0';" \
		" read;\"", \
		d, useragent, r, cookiefile, NULL \
	} \
}

/* DOWNLOAD(URI, referer) */
//#define DOWNLOAD(u, r) { \
//        .v = (const char *[]){ "st", "-e", "/bin/sh", "-c",\
//             "curl -g -L -J -O -A \"$1\" -b \"$2\" -c \"$2\"" \
//             " -e \"$3\" \"$4\"; read", \
//             "surf-download", useragent, cookiefile, r, u, NULL \
//        } \
//}

/* PLUMB(URI) */
/* This called when some URI which does not begin with "about:",
 * "http://" or "https://" should be opened.
 */
#define PLUMB(u) {\
        .v = (const char *[]){ "/bin/sh", "-c", \
             "xdg-open \"$0\"", u, NULL \
        } \
}

/* VIDEOPLAY(URI) */
#define VIDEOPLAY(u) {\
        .v = (const char *[]){ "/bin/sh", "-c", \
             "mpv --really-quiet \"$0\"", u, NULL \
        } \
}

/* NEWTAB */
#define NEWTAB() {\
        .v = (const char *[]){ "/bin/sh", "-c", \
             "surf", NULL \
        } \
}

static SiteSpecific styles[] = {
	/* regexp               file in $styledir */
	{ ".*wikipedia.org.*",  "wikipedia.css" },
	{ ".*stackoverflow.com", "stackoverflow.css" },
	{ ".*docs.rs", "rustdocs.css" },
	{ ".*",                 "default.css" },
};
static SiteSpecific certs[] = {
	/* regexp               file in $certdir */
	{ "://suckless\\.org/", "suckless.org.crt" },
};

#define MODKEY GDK_CONTROL_MASK

static Key keys[] = {
	/* modifier              keyval          function    arg */
	{ GDK_CONTROL_MASK,                     GDK_KEY_l,      spawn,      SETPROP_URI("_SURF_URI", "_SURF_GO", PROMPT_GO) },

	{ 0,                     GDK_KEY_o,      externalpipe,      { .v = piped_open } },
	{ GDK_SHIFT_MASK|0,      GDK_KEY_o,      externalpipe,      { .v = piped_opennew } },

	//{ 0,                     GDK_KEY_slash,  spawn,      SETPROP("_SURF_FIND", "_SURF_FIND", PROMPT_FIND) },
	{ 0,                     GDK_KEY_slash,      externalpipe, { .v = piped_find} },

	{ 0,                     GDK_KEY_i,      insert,     { .i = 1 } },
	{ 0,                     GDK_KEY_Escape, insert,     { .i = 0 } },

	{ 0,                     GDK_KEY_s,      stop,       { 0 } },
	{ GDK_SHIFT_MASK|0,      GDK_KEY_r,      reload,     { .i = 1 } },
	{ 0,                     GDK_KEY_r,      reload,     { .i = 0 } },

	{ 0,                     GDK_KEY_l,      navigate,   { .i = +1 } },
	{ 0,                     GDK_KEY_h,      navigate,   { .i = -1 } },

	{ GDK_SHIFT_MASK|0,      GDK_KEY_BackSpace, navigate,   { .i = +1 } },
	{ 0,                     GDK_KEY_BackSpace, navigate,   { .i = -1 } },

	{ 0,                     GDK_KEY_j,      scrollv,    { .i = +10 } },
	{ 0,                     GDK_KEY_k,      scrollv,    { .i = -10 } },

	{ 0,                     GDK_KEY_c,      toggletitle,    {0} },
	{ MODKEY,                GDK_KEY_c,      toggletitle,    {0} },

	{ 0,                     GDK_KEY_b,      spawn,      LOADBMK("_SURF_URI", "_SURF_GO", PROMPT_GO)},
	{ GDK_SHIFT_MASK|0,      GDK_KEY_b,      spawn,      ADDBMK},
	{ 0,       		 GDK_KEY_m,      spawn,      URLHANDLER},
	{ GDK_SHIFT_MASK|0,      GDK_KEY_m,      externalpipe,      { .v = linkselect_urlhandler}},

	{ 0,                     GDK_KEY_bracketleft,      scrollv,    { .i = -50 } },
	{ 0,                     GDK_KEY_bracketright,      scrollv,    { .i = +50 } },
	{ MODKEY,                GDK_KEY_f,      scrollv,    { .i = +100 } },
	{ MODKEY,                GDK_KEY_b,      scrollv,    { .i = -100 } },
	//{ 0,      GDK_KEY_8,      spawn,    XDOKEYMULTI("Page_Up") },
	//{ 0,      GDK_KEY_9,      spawn,   XDOKEYMULTI("Page_Down") },

	{ MODKEY,                GDK_KEY_t,  spawn,           NEWTAB()},
	{ MODKEY,                GDK_KEY_w,  destroyclient,           {}},

	{ 0,                     GDK_KEY_minus,  zoom,       { .i = -1 } },
	{ 0,                     GDK_KEY_equal,  zoom,       { .i = +1 } },
	{ GDK_SHIFT_MASK|0,     GDK_KEY_minus,  zoom,       { .i = -2 } },
	{ GDK_SHIFT_MASK|0,     GDK_KEY_equal,  zoom,       { .i = +2 } },

	{ 0,                     GDK_KEY_0,  zoom,       { .i = 0 } },

	{ 0,                     GDK_KEY_n,      find,       { .i = +1 } },
	{ GDK_SHIFT_MASK|0,      GDK_KEY_n,      find,       { .i = -1 } },

	{ 0,                     GDK_KEY_w,      toggleinspector, { 0 } },
	{ GDK_SHIFT_MASK|0,      GDK_KEY_w,      externalpipe, { .v = editbuffer } },
	{ 0,			 GDK_KEY_u,      externalpipe, { .v = linkselect_curwin } },
	{ GDK_SHIFT_MASK|0,	 GDK_KEY_u,      externalpipe, { .v = linkselect_newwin } },

	{ 0,			 GDK_KEY_g,      externalpipe, { .v = codeblock_yank } },


	{ 0,                     GDK_KEY_t,      showcert,   { 0 } },

	{ 0,                     GDK_KEY_p,      clipboard,  { .i = 1 } },
	{ 0,                     GDK_KEY_y,      clipboard,  { .i = 0 } },
	{ GDK_SHIFT_MASK|0,	 GDK_KEY_y,      externalpipe, { .v = linkselect_yank } },

	{ MODKEY|GDK_SHIFT_MASK,      GDK_KEY_i,      externalpipe, { .v = string_yank } },
	{ MODKEY|GDK_SHIFT_MASK,      GDK_KEY_o,      externalpipe, { .v = image_select } },


	{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_p,      print,      { 0 } },
	{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_a,      togglecookiepolicy, { 0 } },
	{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_c,      toggle,     { .i = CaretBrowsing } },
	{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_f,      toggle,     { .i = FrameFlattening } },
	{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_g,      toggle,     { .i = Geolocation } },
	{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_s,      toggle,     { .i = JavaScript } },
	{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_i,      toggle,     { .i = LoadImages } },
	{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_v,      toggle,     { .i = Plugins } },
	{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_b,      toggle,     { .i = ScrollBars } },
	{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_t,      toggle,     { .i = StrictTLS } },
	{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_b,      toggle,     { .i = Style } },

	//{ MODKEY,                GDK_KEY_space,  scrollv,    { .i = +50 } },
	//{ MODKEY,                GDK_KEY_slash,  spawn,      SETPROP("_SURF_FIND", "_SURF_FIND", PROMPT_FIND) },
	//{ MODKEY,                GDK_KEY_g,      spawn,      SETPROP("_SURF_URI", "_SURF_GO", PROMPT_GO) },
	//{ MODKEY,                GDK_KEY_c,      stop,       { 0 } },
	//{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_r,      reload,     { .i = 1 } },
	//{ MODKEY,                GDK_KEY_r,      reload,     { .i = 0 } },
	//{ MODKEY,                GDK_KEY_l,      navigate,   { .i = +1 } },
	//{ MODKEY,                GDK_KEY_h,      navigate,   { .i = -1 } },
	//{ MODKEY,                GDK_KEY_j,      scrollv,    { .i = +10 } },
	//{ MODKEY,                GDK_KEY_k,      scrollv,    { .i = -10 } },
	//{ MODKEY,                GDK_KEY_i,      scrollh,    { .i = +10 } },
	//{ MODKEY,                GDK_KEY_u,      scrollh,    { .i = -10 } },
	//{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_j,      zoom,       { .i = -1 } },
	//{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_k,      zoom,       { .i = +1 } },
	//{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_q,      zoom,       { .i = 0  } },
	//{ MODKEY,                GDK_KEY_minus,  zoom,       { .i = -1 } },
	//{ MODKEY,                GDK_KEY_plus,   zoom,       { .i = +1 } },
	//{ MODKEY,                GDK_KEY_p,      clipboard,  { .i = 1 } },
	//{ MODKEY,                GDK_KEY_y,      clipboard,  { .i = 0 } },
	//{ MODKEY,                GDK_KEY_n,      find,       { .i = +1 } },
	//{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_n,      find,       { .i = -1 } },
	//{ 0,                     GDK_KEY_F11,    togglefullscreen, { 0 } },
	//{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_o,      toggleinspector, { 0 } },

};

/* button definitions */
/* target can be OnDoc, OnLink, OnImg, OnMedia, OnEdit, OnBar, OnSel, OnAny */
static Button buttons[] = {
	/* target       event mask      button  function        argument        stop event */
	{ OnLink,       0,              2,      clicknewwindow, { .i = 0 },     1 },
	{ OnLink,       MODKEY,         2,      clicknewwindow, { .i = 1 },     1 },
	{ OnLink,       MODKEY,         1,      clicknewwindow, { .i = 1 },     1 },
	{ OnAny,        0,              8,      clicknavigate,  { .i = -1 },    1 },
	{ OnAny,        0,              9,      clicknavigate,  { .i = +1 },    1 },
	{ OnMedia,      MODKEY,         1,      clickexternplayer, { 0 },       1 },
};
