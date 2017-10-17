/*
// @(#) dumputmp.c - dump contents of utmp formatted file
*/
# define	_XOPEN_SOURCE	600

# include	<unistd.h>
# include	<time.h>
# include	<paths.h>
# include	<utmp.h>
# include	<arpa/inet.h>
# include	<stdlib.h>
# include	<stdio.h>
# include	<string.h>

enum	{
	ok	= 0,
	err	= -1,
};
enum	{
	false	= 0,
	true	= !false,
};
/*
// macro hack to get max width string format "%.256s"
// from the constants in <utmp.h>
*/
# define	str(s) #s
# define	format_string(n)	"%." str(n) "s"
# define	UT_IDSIZE	4
/*
//-------
*/
# define	DATEFMT		"%Y-%m-%d"
# define	TIMEFMT		"%H:%M:%S"
enum	{
	DATESIZE	= sizeof("2000-01-01_23"),
	TIMESIZE	= sizeof("23:59:59"),
};

/*
// -- map utmp types <-> names
*/
char*	T_NAMES []	= {
	[EMPTY]		= "empty",
	[RUN_LVL]	= "run-level",
	[BOOT_TIME]	= "boot",
	[NEW_TIME]	= "time-new",
	[OLD_TIME]	= "time-old",
	[INIT_PROCESS]	= "init",
	[LOGIN_PROCESS]	= "login",
	[USER_PROCESS]	= "user",
	[DEAD_PROCESS]	= "terminated",
};
enum	{
	N_TYPENAMES	= (sizeof(T_NAMES)/sizeof(T_NAMES[0])),
};

char*	ut_typename (int type) {
	char*	result	= "unknown";
	if (type < N_TYPENAMES && 0 <= type) {
		char*	name	= T_NAMES[type];
		if (name) 
			result	= name;
	}
	return	result;
}
/*
//-------------------------------------------------------
*/
/*
// We will be running this on a number of machines
// and need to print which one
*/
typedef	struct	nodenames{
	char	shortname[UT_HOSTSIZE];
	char	longname [UT_HOSTSIZE]; 
}	NODENAMES;

static	int	get_nodenames (NODENAMES* nn) {
	int	result	= gethostname (nn->longname, sizeof(nn->longname));
	if (result == ok) {
		char*	t	= strcpy (nn->shortname, nn->longname);
		char*	s	= strchr (t, '.');
		if (s) {
			*s	= '\0';
		}
	}
	return	result;
}
/*
//-------------------------------------------------------
*/
/*
// Take a string 'd' in DATEFMT " " TIMEFMT and
// produce time_t seconds since the epoch
// 'd' can be DATEFMT | TIMEFMT | DATEFMT " " TIMEFMT
// with the default date (today) and default time (00:00:00)
*/

time_t	date_convert (const char* d) {
	time_t	result	= 0;
	struct	tm	start;
	char	datestr[2*(DATESIZE+TIMESIZE)];

	strncpy (datestr, d, sizeof(datestr));

	char*	next	= strptime (datestr, DATEFMT, &start);
	if (next) {	/* matched the DATE pattern */

		/* try the TIME pattern */
		char*	p	= strptime (next, TIMEFMT, &start);
		if (p == 0) {	
			/* no match use the default */
			strncat (datestr, "00:00:00", sizeof(datestr));
		}
	}
	else	{
		/* assume some TIME today */
		/* calculate today */
		time_t	now	= time (0);
		struct	tm*	tm	= localtime(&now);
		strftime (datestr, sizeof(datestr), DATEFMT " ", tm);
		/* append time */
		strncat (datestr, d, sizeof(datestr));
	}
	/* finally do the conversion */
	next	= strptime (datestr, DATEFMT " " TIMEFMT, &start);
		
	if (next) {	 
		time_t	t	= mktime(&start);
		if (t != (time_t)(-1))
			result	= t;
	}
	return	result;
}
/*
//-------------------------------------------------------
*/
static	void	Usage () {
	fprintf (stderr, "Usage: dumpwtmp [-f utmpfile] [-t type [-t type]..] [-O output_delimiter] [-a 'YYYY-MM-DD HH:MM:SS']\n");
	exit (EXIT_FAILURE);
}
void	print_utmpentry (NODENAMES thisnode, struct utmp* utmp, int sep);

/*
// Use a bitset to keep the list of utmp record type we are interested in.
*/
int	ut_type_is_in (int type, unsigned set) { 
	int	result	= false;
	if ((1<<type) & set)
		result	= true;
	return	result; 
}
unsigned	ut_type_insert (int type, unsigned* setp) {
	unsigned	result	= (1 << type);
	unsigned	set	= *setp | result;
	*setp	= set;
	return	result;
}
/*
// Parse the -t type1,type2,... suboptions
*/
int	process_types (char* optstr, unsigned* setp) {
	char*	dummy	= 0;
	char*	subopts	= optstr;
	while (*subopts != '\0') {
		int	type	= getsubopt (&subopts, T_NAMES, &dummy);
		if (type != EOF) {
			ut_type_insert (type, setp);
		}
	}
	return	ok;
}
			
main (int argc, char* argv[]) {
	int	opt		= EOF;
	int	f_flag		= 0;
	char*	wtmpfile	= _PATH_WTMP;
	unsigned	selected	= EMPTY;

	int	O_flag	= 0;
	int	sep		= ' ';

	int	a_flag	= 0;
	time_t	after	= 0;

	struct	utmp*	utmp	= 0;
	NODENAMES	thisnode = {	.longname = "localhost.",
					.shortname = "localhost" };

	get_nodenames(&thisnode);

	while ((opt = getopt (argc, argv, "f:t:O:a:")) != EOF) {
		switch (opt) {
		case	'f':
			if (f_flag++)
				Usage ();
			wtmpfile	= optarg;
		break;
		case	't':
			process_types (optarg, &selected);
		break;
		case	'O':
			if (O_flag++)
				Usage ();
			sep	= optarg[0];
		break;
		case	'a':
			if (a_flag++)
				Usage ();
			after	= date_convert (optarg);
		break;
		default:
			Usage ();
		}
	}
	if (selected == EMPTY)
		selected	= ~EMPTY;
				
	utmpname (wtmpfile);
	setutxent();

	while ((utmp = getutent()) != 0) {
		time_t	t	= (time_t) utmp->ut_tv.tv_sec;
		int	ut_type	= utmp->ut_type;

		if ( ut_type_is_in(ut_type, selected) && t >= after) {
			print_utmpentry (thisnode, utmp, sep); 
		}
	}
	endutent();
	exit (EXIT_SUCCESS);
}

void	print_utmpentry (NODENAMES thisnode, struct utmp* utmp, int sep) {

	time_t	t	= (time_t) utmp->ut_tv.tv_sec;
	struct tm*	tm 	= localtime (&t);

	char	ut_date [DATESIZE];
	char	ut_hhmmss [TIMESIZE];

	int32_t	ip	= utmp->ut_addr_v6[0];
	char*	host	= ip==0 ? thisnode.longname : utmp->ut_host;
	
	char 		addr[INET_ADDRSTRLEN];
	inet_ntop (AF_INET, &ip, addr, sizeof(addr));


	strftime (ut_date, sizeof(ut_date), DATEFMT, tm);	
	strftime (ut_hhmmss, sizeof(ut_hhmmss), TIMEFMT, tm);	


	printf ("%s%c", thisnode.shortname, sep);
	printf ("%s%c", ut_typename (utmp->ut_type), sep);
	printf ("%lu%c", utmp->ut_pid, sep);
	printf (format_string(UT_LINESIZE) "%c", utmp->ut_line, sep);
	printf (format_string(UT_IDSIZE) "%c", utmp->ut_id, sep);
	printf (format_string(UT_NAMESIZE) "%c", utmp->ut_user, sep);
	printf ("%d:", utmp->ut_exit.e_termination);
	printf ("%d%c", utmp->ut_exit.e_exit, sep);

	printf ("%s%c", addr, sep);
	printf (format_string(UT_HOSTSIZE) "%c", host, sep);
	printf ("%s%c", ut_date, sep);
	printf ("%s%c", ut_hhmmss, sep);
	printf ("%ld\n", utmp->ut_tv.tv_sec);
}
