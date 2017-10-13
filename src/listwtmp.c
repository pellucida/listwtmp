/*
// @(#) listwtmp.c - list the utmp file
*/
# include	<unistd.h>
# include	<time.h>
# include	<paths.h>
# include	<utmpx.h>
# include	<arpa/inet.h>
# include	<stdlib.h>
# include	<stdio.h>
# include	<string.h>

enum	{
	UT_HOSTSIZE	= sizeof(((struct utmpx*)(0))->ut_host),
	UT_NAMESIZE	= sizeof(((struct utmpx*)(0))->ut_user),
	UT_TIMESIZE	= sizeof("2000-01-01_23:22:21.01"),
};

enum	{
	ok	= 0,
	err	= -1,
};
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

static	void	Usage () {
	fprintf (stderr, "Usage: listwtmp [-f utmpfile] [-[W|D|H|M|S] weeks|days|hrs|mins|secs_ago]\n");
	exit (EXIT_FAILURE);
}
main (int argc, char* argv[]) {
	int	opt		= EOF;
	int	f_flag		= 0;
	int	a_flag		= 0;
	char*	ago_str		= 0;
	time_t	since		= 0;
	unsigned long multiplier	= 1;
	char*	wtmpfile	= _PATH_WTMP;

	struct	utmpx*	utmp	= 0;
	struct	tm*	tm	= 0;
	NODENAMES	thisnode	= { .longname = "localhost.", .shortname = "localhost" };
	char	when [UT_TIMESIZE];

	get_nodenames(&thisnode);

	while ((opt = getopt (argc, argv, "hf:S:M:H:D:W:")) != EOF) {
		switch (opt) {
		case	'f':
			if (f_flag++)
				Usage ();
			wtmpfile	= optarg;
		break;
		case	'W':
			multiplier	*= 7;
		case	'D':
			multiplier	*= 24;
		case	'H':
			multiplier	*= 60;
		case	'M':
			multiplier	*= 60;
		case	'S':
			if (a_flag++)
				Usage ();
			ago_str	= optarg;
		break;
		case	'h':
		case	'?':
		default:
			Usage ();
		}
	}
				
	utmpxname (wtmpfile);
	setutxent();
	if (a_flag) {
		time_t	now		= time (0);
		unsigned long long before = strtoull (ago_str, 0, 10);
		since	= now - (before * multiplier);
	}
	while ((utmp = getutxent()) != 0) {
		if (utmp->ut_type == USER_PROCESS) {
			char 		addr[INET_ADDRSTRLEN];
			int32_t	ip	= utmp->ut_addr_v6[0];
			char*	host	= ip==0? thisnode.longname : utmp->ut_host;
			time_t	t	= (time_t) utmp->ut_tv.tv_sec;
			char*	user	= utmp->ut_user;
			char*	line	= utmp->ut_line;

			if (t >= since) {
				
				inet_ntop(AF_INET, &ip, addr, sizeof(addr));
				tm 	= localtime (&t);
				strftime (when, sizeof(when), "%Y-%m-%d", tm);	

				if (user[0] == '\0') { // logout on this line
					continue;
				}
				else	{
					printf ("%s %s %s %s %s %s\n",
							thisnode.shortname, user, line,
							host, addr, when);
				}
			}
		}
	}
	endutxent();
	exit (0);
}
