/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Thanks to abaddon for proof-reading our comm.c and pointing out bugs.  *
 *  Any remaining bugs are, of course, our work, not his.  :)              *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/*
 * This file contains all of the OS-dependent stuff:
 *   startup, signals, BSD sockets for tcp/ip, i/o, timing.
 *
 * The data flow for input is:
 *    Game_loop ---> Read_from_descriptor ---> Read
 *    Game_loop ---> Read_from_buffer
 *
 * The data flow for output is:
 *    Game_loop ---> Process_Output ---> Write_to_descriptor -> Write
 *
 * The OS-dependent functions are Read_from_descriptor and Write_to_descriptor.
 * -- Furey  26 Jan 1993
 */

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "merc.h"

/*
 * Malloc debugging stuff.
 */
#if defined(sun)
#undef MALLOC_DEBUG
#endif

#if defined(MALLOC_DEBUG)
#include <malloc.h>
extern	int	malloc_debug	args( ( int  ) );
extern	int	malloc_verify	args( ( void ) );
#endif

/*
 * Signal handling.
 * Apollo has a problem with __attribute(atomic) in signal.h,
 *   I dance around it.
 */
#if defined(apollo)
#define __attribute(x)
#endif

#if defined(unix) || defined(linux)
#include <signal.h>
#endif

#if defined(apollo)
#undef __attribute
#endif

/*
 * Socket and TCP/IP stuff.
 */
#if	defined(macintosh) || defined(MSDOS)
const	char	echo_off_str	[] = { '\0' };
const	char	echo_on_str	[] = { '\0' };
const	char 	go_ahead_str	[] = { '\0' };
#endif

/* I'm going to go through and remove most of this shit we don't need */
#if	defined(linux) || defined(unix)
#include <fcntl.h>
#include <netdb.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <arpa/telnet.h>
const   char    echo_off_str    [] = { IAC, WILL, TELOPT_ECHO, '\0' };
const   char    echo_on_str     [] = { IAC, WONT, TELOPT_ECHO, '\0' };
const   char    go_ahead_str    [] = { IAC, GA, '\0' };
#endif

/*
 * OS-dependent declarations.
 */
#if	defined(_AIX)
#include <sys/select.h>
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
void	bzero		args( ( char *b, int length ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
int	listen		args( ( int s, int backlog ) );
int	setsockopt	args( ( int s, int level, int optname, void *optval,
			    int optlen ) );
int	socket		args( ( int domain, int type, int protocol ) );
#endif

#if	defined(apollo)
#include <unistd.h>
void	bzero		args( ( char *b, int length ) );
#endif

#if	defined(__hpux)
int	accept		args( ( int s, void *addr, int *addrlen ) );
int	bind		args( ( int s, const void *addr, int addrlen ) );
void	bzero		args( ( char *b, int length ) );
int	getpeername	args( ( int s, void *addr, int *addrlen ) );
int	getsockname	args( ( int s, void *name, int *addrlen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
int	listen		args( ( int s, int backlog ) );
int	setsockopt	args( ( int s, int level, int optname,
 				const void *optval, int optlen ) );
int	socket		args( ( int domain, int type, int protocol ) );
#endif

#if     defined(interactive)
#include <net/errno.h>
#include <sys/fcntl.h>
#endif

#if	defined(linux)
#include <unistd.h>
#endif

#if	defined(macintosh)
#include <console.h>
#include <fcntl.h>
#include <unix.h>
struct	timeval
{
	time_t	tv_sec;
	time_t	tv_usec;
};
static	long			theKeys	[4];

int	gettimeofday		args( ( struct timeval *tp, void *tzp ) );
#endif

#if	defined(MIPS_OS)
extern	int		errno;
#endif

#if	defined(MSDOS)
int	gettimeofday	args( ( struct timeval *tp, void *tzp ) );
int	kbhit		args( ( void ) );
#endif

#if	defined(NeXT)
int	close		args( ( int fd ) );
int	fcntl		args( ( int fd, int cmd, int arg ) );
#if	!defined(htons)
u_short	htons		args( ( u_short hostshort ) );
#endif
#if	!defined(ntohl)
u_long	ntohl		args( ( u_long hostlong ) );
#endif
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

#if	defined(sequent)
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
int	close		args( ( int fd ) );
int	fcntl		args( ( int fd, int cmd, int arg ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
#if	!defined(htons)
u_short	htons		args( ( u_short hostshort ) );
#endif
int	listen		args( ( int s, int backlog ) );
#if	!defined(ntohl)
u_long	ntohl		args( ( u_long hostlong ) );
#endif
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int	setsockopt	args( ( int s, int level, int optname, caddr_t optval,
			    int optlen ) );
int	socket		args( ( int domain, int type, int protocol ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

/*
 * This includes Solaris SYSV as well
 */
#if defined(sun)
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
void	bzero		args( ( char *b, int length ) );
int	close		args( ( int fd ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
int	listen		args( ( int s, int backlog ) );
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
#if defined(SYSV)
int     setsockopt      args( ( int s, int level, int optname,
			    const char *optval, int optlen ) );
#else
int	setsockopt	args( ( int s, int level, int optname, void *optval,
			    int optlen ) );
#endif
int	socket		args( ( int domain, int type, int protocol ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

#if defined(ultrix)
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
void	bzero		args( ( char *b, int length ) );
int	close		args( ( int fd ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
int	listen		args( ( int s, int backlog ) );
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int	setsockopt	args( ( int s, int level, int optname, void *optval,
			    int optlen ) );
int	socket		args( ( int domain, int type, int protocol ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

/*
 * Global variables.
 */
DESCRIPTOR_DATA *   descriptor_free;	/* Free list for descriptors	*/
DESCRIPTOR_DATA *   descriptor_list;	/* All open descriptors		*/
DESCRIPTOR_DATA *   d_next;		/* Next descriptor in loop	*/
FILE *		    fpReserve;		/* Reserved file handle		*/
bool		    god;		/* All new chars are gods!	*/
bool		    merc_down;		/* Shutdown			*/
bool		    wizlock;		/* Game is wizlocked		*/
char		    str_boot_time[MAX_INPUT_LENGTH];
time_t		    current_time;	/* Time of this pulse		*/

/*
 * OS-dependent local functions.
 */
#if defined(macintosh) || defined(MSDOS)
void	game_loop_mac_msdos	args( ( void ) );
bool	read_from_descriptor	args( ( DESCRIPTOR_DATA *d ) );
bool	write_to_descriptor	args( ( int desc, char *txt, int length ) );
#endif

#if defined(unix) || defined(linux)
void	game_loop_unix		args( ( int control ) );
int	init_socket		args( ( int port ) );
void	new_descriptor		args( ( int control ) );
bool	read_from_descriptor	args( ( DESCRIPTOR_DATA *d ) );
bool	write_to_descriptor	args( ( int desc, char *txt, int length ) );
#endif

/*
 * Other local functions (OS-independent).
 */
bool	check_parse_name	args( ( char *name ) );
bool	check_reconnect		args( ( DESCRIPTOR_DATA *d, char *name,
				    bool fConn ) );
bool	check_playing		args( ( DESCRIPTOR_DATA *d, char *name ) );
int	main			args( ( int argc, char **argv ) );
void	nanny			args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool	process_output		args( ( DESCRIPTOR_DATA *d, bool fPrompt ) );
void	read_from_buffer	args( ( DESCRIPTOR_DATA *d ) );
void	stop_idling		args( ( CHAR_DATA *ch ) );
void    bust_a_prompt           args( ( CHAR_DATA *ch ) );

//int     port, control;
int     game_port = 1234, control;
char    * game_port_str = "1234";

#if defined(unix) || defined(linux)
static void signal_handler(int signo) {
    pid_t p;

    switch (signo) {
        case SIGPIPE:
            log_string("lol sigpipe");
            /*
             * old code was as follows
             * signal(SIGPIPE, SIG_IGN); which just ignores SIGPIPE
             * I'm assuming we're just ignore it as well here by doing
             * nothing at all when we catch it.  Or maybe that's not how
             * it works?
             */
        break;
        case SIGINT:
            log_string("lol sigint");
        break;
        case SIGCHLD:
            log_string("lol sigchld");
            do {
                p = waitpid(-1, NULL, WNOHANG);
            } while (p != (pid_t)0 && p != (pid_t)-1);
        break;
        case SIGTERM:
            log_string("lol sigterm");
        break;
    }
}

void init_signals(void) {
    struct sigaction sigact;

    sigact.sa_handler = signal_handler;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = SA_NOCLDSTOP|SA_RESTART|SA_SIGINFO|SA_NOCLDWAIT;
    sigaction(SIGPIPE, &sigact, (struct sigaction *) NULL);
    sigaction(SIGINT, &sigact, (struct sigaction *) NULL);
    sigaction(SIGCHLD, &sigact, (struct sigaction *) NULL);
    sigaction(SIGTERM, &sigact, (struct sigaction *) NULL);
}
#endif

int main( int argc, char *argv[] ) {
    struct timeval now_time;
    bool fCopyOver = FALSE;

    /*
     * Memory debugging if needed.
     */
#if defined(MALLOC_DEBUG)
    malloc_debug( 2 );
#endif

    /*
     * Init time.
     */
    gettimeofday( &now_time, NULL );
    current_time = (time_t) now_time.tv_sec;
    strcpy( str_boot_time, ctime( &current_time ) );

    /*
     * Macintosh console initialization.
     */
#if defined(macintosh)
    console_options.nrows = 31;
    csetmode( C_RAW, stdin );
    cshow( stdout );
    cecho2file( "log file", 1, stderr );
#endif

    /*
     * Reserve one channel for our use.
     */
    if ((fpReserve = fopen(NULL_FILE, "r")) == NULL) {
        perror( NULL_FILE );
        exit( 1 );
    }

    /*
     * Get the port number.
     */
    if ( argc > 1 ) {
	    if ( !is_number( argv[1] ) ) {
	        fprintf( stderr, "Usage: %s [port #]\n", argv[0] );
	        exit( 1 );
	    } else if ( ( game_port = atoi( argv[1] ) ) <= 1024 ) {
            fprintf( stderr, "Port number must be above 1024.\n" );
            exit( 1 );
	    }

        if (argv[2] && argv[2][0]) {
            fCopyOver = TRUE;
            control = atoi(argv[3]);
        } else
            fCopyOver = FALSE;
    }

    /*
     * Run the game.
     */
#if defined(macintosh) || defined(MSDOS)
    boot_db( );
    log_string( "Merc is ready to rock." );
    game_loop_mac_msdos( );
#endif

#if defined(unix) || defined(linux)
    if (!fCopyOver)
        control = init_socket( game_port );
    boot_db(fCopyOver);
    log_string("Merc is ready to rock on port %d.", game_port);
    game_loop_unix( control );
    close( control );
#endif

    /*
     * That's all, folks.
     */
    log_string( "Normal termination of game." );
    return EXIT_SUCCESS;
}

#if defined(unix) || defined(linux)
int init_socket(int port) {
    int keepalive_time = 120, keepalive = 1, keepalive_count = 10, keepalive_interval = 30, status = 0, sock = -1, x = 1;
    struct addrinfo hints, *service, *ptr;

    /*
     * Use AF_INET or AF_INET6 instead of AF_UNSPEC if you want to force either of those 
     */
    memset(&hints, 0, sizeof hints);
    hints.ai_family     = AF_UNSPEC;
    hints.ai_socktype   = SOCK_STREAM;
    hints.ai_flags      = AI_PASSIVE;

    if ((status = getaddrinfo(NULL, game_port_str, &hints, &service)) != 0) {
        char error[2046]; /* Not sure if this is big enough? */
        snprintf(error, sizeof error, "getaddrinfo(): %s\n", gai_strerror(status));
        perror(error);
        exit(EXIT_FAILURE);
    }

    for (ptr = service; ptr != NULL; ptr = ptr->ai_next) {
        if ((sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) < 0) {
            perror("Failed to create socket");
            exit(EXIT_FAILURE);
        }

        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *) &x, sizeof x) < 0) {
            perror("Failed to create socket");
            close(sock);
            exit(EXIT_FAILURE);
        }

#if defined(SO_DONTLINGER) && !defined(SYSV)
        {
            struct	linger	ld;

            ld.l_onoff  = 1;
            ld.l_linger = 1000;

            if (setsockopt(sock, SOL_SOCKET, SO_DONTLINGER, (char *) &ld, sizeof(ld)) < 0) {
                perror("Init_socket: SO_DONTLINGER");
                close(sock);
                exit(EXIT_FAILURE);
            }
        }
#endif

        if (bind(sock, ptr->ai_addr, ptr->ai_addrlen) < 0) {
            perror("Failed to bind to address");
            close(sock);
            exit(EXIT_FAILURE);
        }

        /*
         * I believe I had issues trying to bind IPv4 addresses to the same port previously
         * as I hadn't allocated any IPv6 addresses to my server.  I broke out of the loop
         * after binding the first address which "fixed" the issue.
         * However, when you have IPv6 enabled as well but still break out, you'll only
         * listen on your IPv4 address which was causing issues because everything expects
         * IPv6 to be working(At least, if your domain has an AAAA record as well as an A record).
         * GNU/Linux and the BSDs support dual-stack sockets so we can listen to IPv4/6 on the same
         * port.  So, we'll break out if there is nothing next which should fix if you server only
         * supports IPv4 but will still allow IPv6 on servers which have it enabled.
         * 
         * /proc/sys/net/ipv6/bindv6only should report 0 if your server will listen on ipv4 as well
         * when creating an IPv6 socket.
         * You can also use setsockopt to enable or disable this feature if you can't change it
         * at the system level.
         */
        if (ptr->ai_next == NULL)
            break;
    }

    /* Have to make sure to clean up after getaddrinfo */
    freeaddrinfo(service);

    /*
     * I've read the backlog max is 128(Not from Beej's Guide), Beej's networking
     * guide starts with ten and says to increase if we experience connection
     * refused errors...
     * I've also read that it's silently limited to 5 in C...
     * Most of this was years ago, I'm sure this is fine.
     */
    if (listen(sock, 10) < 0) {
        perror("Init_socket: listen");
        close(sock);
        exit(EXIT_FAILURE);
    }

    /* Set keep-alive so clients with crappy routers don't get disconnected while idle */
    if (setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(int)) == -1) {
        perror("Error in setsockopt(keepalive)");
        close(sock);
        exit(EXIT_FAILURE);
    }

    setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepalive_time, sizeof keepalive_time);
    setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &keepalive_count, sizeof keepalive_count);
    setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepalive_interval, sizeof keepalive_interval);
    return sock;
}
#endif

#if defined(macintosh) || defined(MSDOS)
void game_loop_mac_msdos( void )
{
    struct timeval last_time;
    struct timeval now_time;
    static DESCRIPTOR_DATA dcon;

    gettimeofday( &last_time, NULL );
    current_time = (time_t) last_time.tv_sec;

    /*
     * New_descriptor analogue.
     */
    dcon.descriptor	= 0;
    dcon.connected	= CON_GET_NAME;
    dcon.host		= str_dup( "localhost" );
    dcon.outsize	= 2000;
    dcon.outbuf		= alloc_mem( dcon.outsize );
    dcon.next		= descriptor_list;
    descriptor_list	= &dcon;

    /*
     * Send the greeting.
     */
    {
	extern char * help_greeting;
	if ( help_greeting[0] == '.' )
	    write_to_buffer( &dcon, help_greeting+1, 0 );
	else
	    write_to_buffer( &dcon, help_greeting  , 0 );
    }

    /* Main loop */
    while ( !merc_down )
    {
	DESCRIPTOR_DATA *d;

	/*
	 * Process input.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next	= d->next;
	    d->fcommand	= FALSE;

#if defined(MSDOS)
	    if ( kbhit( ) )
#endif
	    {
		if ( d->character != NULL )
		    d->character->timer = 0;
		if ( !read_from_descriptor( d ) )
		{
		    if ( d->character != NULL )
			save_char_obj( d->character );
		    d->outtop	= 0;
		    close_socket( d );
		    continue;
		}
	    }

	    if ( d->character != NULL && d->character->wait > 0 )
	    {
		--d->character->wait;
		continue;
	    }

	    read_from_buffer( d );
	    if ( d->incomm[0] != '\0' )
	    {
		d->fcommand	= TRUE;
		stop_idling( d->character );

		if ( d->connected == CON_PLAYING )
		    if ( d->showstr_point )
		        show_string( d, d->incomm );
		    else
		        interpret( d->character, d->incomm );
		else
		    nanny( d, d->incomm );

		d->incomm[0]	= '\0';
	    }
	}



	/*
	 * Autonomous game motion.
	 */
	update_handler( );



	/*
	 * Output.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next = d->next;

	    if ( ( d->fcommand || d->outtop > 0 ) )
	    {
		if ( !process_output( d, TRUE ) )
		{
		    if ( d->character != NULL )
			save_char_obj( d->character );
		    d->outtop	= 0;
		    close_socket( d );
		}
	    }
	}



	/*
	 * Synchronize to a clock.
	 * Busy wait (blargh).
	 */
	now_time = last_time;
	for ( ; ; )
	{
	    int delta;

#if defined(MSDOS)
	    if ( kbhit( ) )
#endif
	    {
		if ( dcon.character != NULL )
		    dcon.character->timer = 0;
		if ( !read_from_descriptor( &dcon ) )
		{
		    if ( dcon.character != NULL )
			save_char_obj( d->character );
		    dcon.outtop	= 0;
		    close_socket( &dcon );
		}
#if defined(MSDOS)
		break;
#endif
	    }

	    gettimeofday( &now_time, NULL );
	    delta = ( now_time.tv_sec  - last_time.tv_sec  ) * 1000 * 1000
		  + ( now_time.tv_usec - last_time.tv_usec );
	    if ( delta >= 1000000 / PULSE_PER_SECOND )
		break;
	}
	last_time    = now_time;
	current_time = (time_t) last_time.tv_sec;
    }

    return;
}
#endif



#if defined(unix) || defined(linux)
void game_loop_unix( int control ) {
    static struct timeval null_time;
    struct timeval last_time;

    init_signals();
    gettimeofday(&last_time, NULL);
    current_time = (time_t) last_time.tv_sec;

    /* Main loop */
    while (!merc_down) {
        fd_set in_set;
        fd_set out_set;
        fd_set exc_set;
        DESCRIPTOR_DATA *d;
        int maxdesc;

#if defined(MALLOC_DEBUG)
	if ( malloc_verify( ) != 1 )
	    abort( );
#endif

	/*
	 * Poll all active descriptors.
	 */
	FD_ZERO( &in_set  );
	FD_ZERO( &out_set );
	FD_ZERO( &exc_set );
	FD_SET( control, &in_set );
	maxdesc	= control;
	for ( d = descriptor_list; d; d = d->next )
	{
	    maxdesc = UMAX( maxdesc, d->descriptor );
	    FD_SET( d->descriptor, &in_set  );
	    FD_SET( d->descriptor, &out_set );
	    FD_SET( d->descriptor, &exc_set );
	}

	if ( select( maxdesc+1, &in_set, &out_set, &exc_set, &null_time ) < 0 )
	{
	    perror( "Game_loop: select: poll" );
	    exit( 1 );
	}

	/*
	 * New connection?
	 */
	if ( FD_ISSET( control, &in_set ) )
	    new_descriptor( control );

	/*
	 * Kick out the freaky folks.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next = d->next;   
	    if ( FD_ISSET( d->descriptor, &exc_set ) )
	    {
		FD_CLR( d->descriptor, &in_set  );
		FD_CLR( d->descriptor, &out_set );
		if ( d->character )
		    save_char_obj( d->character );
		d->outtop	= 0;
		close_socket( d );
	    }
	}

	/*
	 * Process input.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next	= d->next;
	    d->fcommand	= FALSE;

	    if ( FD_ISSET( d->descriptor, &in_set ) )
	    {
		if ( d->character != NULL )
		    d->character->timer = 0;
		if ( !read_from_descriptor( d ) )
		{
		    FD_CLR( d->descriptor, &out_set );
		    if ( d->character != NULL )
			save_char_obj( d->character );
		    d->outtop	= 0;
		    close_socket( d );
		    continue;
		}
	    }

	    if ( d->character != NULL && d->character->wait > 0 )
	    {
		--d->character->wait;
		continue;
	    }

	    read_from_buffer( d );
	    if ( d->incomm[0] != '\0' )
	    {
		d->fcommand	= TRUE;

		if (d->pProtocol != NULL)
		    d->pProtocol->WriteOOB = 0;

		stop_idling( d->character );

		if ( d->connected == CON_PLAYING )
		    if ( d->showstr_point )
		        show_string( d, d->incomm );
		    else
		        interpret( d->character, d->incomm );
		else
		    nanny( d, d->incomm );

		d->incomm[0]	= '\0';
	    }
	}



	/*
	 * Autonomous game motion.
	 */
	update_handler( );



	/*
	 * Output.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next = d->next;

	    if ( ( d->fcommand || d->outtop > 0 )
	    &&   FD_ISSET(d->descriptor, &out_set) )
	    {
		if ( !process_output( d, TRUE ) )
		{
		    if ( d->character != NULL )
			save_char_obj( d->character );
		    d->outtop	= 0;
		    close_socket( d );
		}
	    }
	}



	/*
	 * Synchronize to a clock.
	 * Sleep( last_time + 1/PULSE_PER_SECOND - now ).
	 * Careful here of signed versus unsigned arithmetic.
	 */
	{
	    struct timeval now_time;
	    long secDelta;
	    long usecDelta;

	    gettimeofday( &now_time, NULL );
	    usecDelta	= ((int) last_time.tv_usec) - ((int) now_time.tv_usec)
			+ 1000000 / PULSE_PER_SECOND;
	    secDelta	= ((int) last_time.tv_sec ) - ((int) now_time.tv_sec );
	    while ( usecDelta < 0 )
	    {
		usecDelta += 1000000;
		secDelta  -= 1;
	    }

	    while ( usecDelta >= 1000000 )
	    {
		usecDelta -= 1000000;
		secDelta  += 1;
	    }

	    if ( secDelta > 0 || ( secDelta == 0 && usecDelta > 0 ) )
	    {
		struct timeval stall_time;

		stall_time.tv_usec = usecDelta;
		stall_time.tv_sec  = secDelta;
		if ( select( 0, NULL, NULL, NULL, &stall_time ) < 0 )
		{
		    perror( "Game_loop: select: stall" );
		    exit( 1 );
		}
	    }
	}

	gettimeofday( &last_time, NULL );
	current_time = (time_t) last_time.tv_sec;
    }

    return;
}
#endif

void init_descriptor(DESCRIPTOR_DATA *dnew, int desc) {
    static DESCRIPTOR_DATA d_zero;

    *dnew       = d_zero;
    dnew->descriptor    = desc;
    dnew->connected = CON_GET_NAME;
    dnew->showstr_head  = NULL;
    dnew->showstr_point = NULL;
    dnew->outsize   = 2000;
    dnew->outbuf    = alloc_mem( dnew->outsize );
    dnew->pProtocol     = ProtocolCreate();
}

#if defined(unix) || defined(linux)
void new_descriptor( int control )
{
	struct sockaddr_storage sock;
    DESCRIPTOR_DATA *dnew;
    BAN_DATA *pban;
    int desc;
    socklen_t size;

    size = sizeof(sock);
    getsockname( control, (struct sockaddr *) &sock, &size );
    if ((desc = accept(control, (struct sockaddr *) &sock, &size)) < 0) {
        perror( "New_descriptor: accept" );
        return;
    }

#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif

    if (fcntl(desc, F_SETFL, FNDELAY) == -1) {
        perror("New_descriptor: fcntl: FNDELAY");
        return;
    }

    /*
     * Cons a new descriptor.
     */
    if (descriptor_free == NULL) {
        dnew            = alloc_perm(sizeof(*dnew));
    } else {
        dnew            = descriptor_free;
        descriptor_free = descriptor_free->next;
    }

    init_descriptor(dnew, desc);

    size = sizeof(sock);
    if (getpeername(desc, (struct sockaddr *) &sock, &size) < 0) {
        perror( "New_descriptor: getpeername" );
        dnew->host = str_dup( "(unknown)" );
    } else {
        char ip[INET6_ADDRSTRLEN], host[NI_MAXHOST];
        /*
         * Would be nice to use inet_ntoa here but it takes a struct arg,
         * which ain't very compatible between gcc and system libraries.
         * What's nice is that it's not 1993...  We can use getnameinfo...
         * Which is what we're going to do.
         * - Turncoat
         */

        /* 
         * We could use a switch or if statement to check for family type and
         * call inet_ntop accordingly or we can just call this twice, first time
         * with the NUMERICHOST flag to get the IP address.
         *
         * I wish we could get both the IP and hostname with one call but...
         * we can't.  We don't really need this we could get rid of it if we
         * want.
         */
        if (getnameinfo((struct sockaddr *) &sock, INET6_ADDRSTRLEN, ip, sizeof ip, NULL, 0, NI_NUMERICHOST) == 0) {
            log_string("Do something with IP information: %s", ip);
        }

        /* Now, we're getting the hostname of the connected client */
        if (getnameinfo((struct sockaddr *) &sock, INET6_ADDRSTRLEN, host, sizeof host, NULL, 0, AI_CANONNAME) == 0) {
            log_string("Host: %s", host);
            if (dnew->host != NULL)
                free_string(dnew->host);

            dnew->host = str_dup(host);
        }
    }

    /*
     * Swiftest: I added the following to ban sites.  I don't
     * endorse banning of sites, but Copper has few descriptors now
     * and some people from certain sites keep abusing access by
     * using automated 'autodialers' and leaving connections hanging.
     *
     * Furey: added suffix check by request of Nickel of HiddenWorlds.
     */
    for (pban = ban_list; pban != NULL; pban = pban->next) {
        if (str_suffix( pban->name, dnew->host)) {
            write_to_descriptor(desc,
            "Your site has been banned from this Mud.\n\r", 0);
            close(desc);
            free_string(dnew->host);
            free_mem(dnew->outbuf, dnew->outsize);
            dnew->next      = descriptor_free;
            descriptor_free = dnew;
            return;
        }
    }

    /*
     * Init descriptor data.
     */
    dnew->next      = descriptor_list;
    descriptor_list = dnew;

    ProtocolNegotiate(dnew);

    /*
     * Send the greeting.
     */
    {
        extern char * help_greeting;
        if ( help_greeting[0] == '.' )
            write_to_buffer( dnew, help_greeting+1, 0 );
        else
            write_to_buffer( dnew, help_greeting  , 0 );
    }

    return;
}
#endif

void close_socket( DESCRIPTOR_DATA *dclose )
{
    CHAR_DATA *ch;

    if ( dclose->outtop > 0 )
	process_output( dclose, FALSE );

    if ( dclose->snoop_by != NULL )
    {
	write_to_buffer( dclose->snoop_by,
	    "Your victim has left the game.\n\r", 0 );
    }

    {
	DESCRIPTOR_DATA *d;

	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->snoop_by == dclose )
		d->snoop_by = NULL;
	}
    }

    if ( ( ch = dclose->character ) != NULL )
    {
	sprintf( log_buf, "Closing link to %s.", ch->name );
	log_string( log_buf );
	if ( dclose->connected == CON_PLAYING )
	{
	    act( "$n has lost $s link.", ch, NULL, NULL, TO_ROOM );
	    ch->desc = NULL;
	}
	else
	{
	    free_char( dclose->character );
	}
    }

    if ( d_next == dclose )
	d_next = d_next->next;   

    if ( dclose == descriptor_list )
    {
	descriptor_list = descriptor_list->next;
    }
    else
    {
	DESCRIPTOR_DATA *d;

	for ( d = descriptor_list; d && d->next != dclose; d = d->next )
	    ;
	if ( d != NULL )
	    d->next = dclose->next;
	else
	    bug( "Close_socket: dclose not found.", 0 );
    }

    close(dclose->descriptor);
    free_string(dclose->host);

    ProtocolDestroy(dclose->pProtocol);

    dclose->next	= descriptor_free;
    descriptor_free	= dclose;
#if defined(MSDOS) || defined(macintosh)
    exit(1);
#endif
    return;
}

bool read_from_descriptor( DESCRIPTOR_DATA *d )
{
    static char read_buf[MAX_PROTOCOL_BUFFER];
    int iStart;

    read_buf[0] = '\0';

    /* Hold horses if pending command already. */
    if ( d->incomm[0] != '\0' )
	return TRUE;

    /* Check for overflow. */
    iStart = 0;
    if (strlen(d->inbuf) >= sizeof(d->inbuf) - 10)
    {
	sprintf( log_buf, "%s input overflow!", d->host );
	log_string( log_buf );
	write_to_descriptor( d->descriptor,
	    "\n\r*** PUT A LID ON IT!!! ***\n\r", 0 );
	return FALSE;
    }

    /* Snarf input. */
#if defined(macintosh)
    for ( ; ; )
    {
	int c;
	c = getc( stdin );
	if ( c == '\0' || c == EOF )
	    break;
	putc( c, stdout );
	if ( c == '\r' )
	    putc( '\n', stdout );
	read_buf[iStart++] = c;
	if ( iStart > sizeof(d->inbuf) - 10 )
	    break;
    }
#endif

#if defined(MSDOS) || defined(unix) || defined(linux)
    for ( ; ; )
    {
	int nRead;

	nRead = read( d->descriptor, read_buf + iStart,
	    sizeof(read_buf) - 10 - iStart );
	if ( nRead > 0 )
	{
	    iStart += nRead;
	    if ( read_buf[iStart-1] == '\n' || read_buf[iStart-1] == '\r' )
		break;
	}
	else if ( nRead == 0 )
	{
	    log_string( "EOF encountered on read." );
	    return FALSE;
	}
	else if ( errno == EWOULDBLOCK )
	    break;
	else
	{
	    perror( "Read_from_descriptor" );
	    return FALSE;
	}
    }
#endif

    read_buf[iStart] = '\0';
    ProtocolInput(d, read_buf, iStart, d->inbuf);
    return TRUE;
}

/*
 * Transfer one line from input buffer to input line.
 */
void read_from_buffer( DESCRIPTOR_DATA *d )
{
    int i, j, k;

    /*
     * Hold horses if pending command already.
     */
    if ( d->incomm[0] != '\0' )
	return;

    /*
     * Look for at least one new line.
     */
    for ( i = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
    {
	if ( d->inbuf[i] == '\0' )
	    return;
    }

    /*
     * Canonical input processing.
     */
    for ( i = 0, k = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
    {
	if ( k >= MAX_INPUT_LENGTH - 2 )
	{
	    write_to_descriptor( d->descriptor, "Line too long.\n\r", 0 );

	    /* skip the rest of the line */
	    for ( ; d->inbuf[i] != '\0'; i++ )
	    {
		if ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
		    break;
	    }
	    d->inbuf[i]   = '\n';
	    d->inbuf[i+1] = '\0';
	    break;
	}

	if ( d->inbuf[i] == '\b' && k > 0 )
	    --k;
	else if ( is_ascii(d->inbuf[i]) && isprint(d->inbuf[i]) )
	    d->incomm[k++] = d->inbuf[i];
    }

    /*
     * Finish off the line.
     */
    if ( k == 0 )
	d->incomm[k++] = ' ';
    d->incomm[k] = '\0';

    /*
     * Deal with bozos with #repeat 1000 ...
     */
    if ( k > 1 || d->incomm[0] == '!' )
    {
    	if ( d->incomm[0] != '!' && strcmp( d->incomm, d->inlast ) )
	{
	    d->repeat = 0;
	}
	else
	{
	    if ( ++d->repeat >= 20 )
	    {
		sprintf( log_buf, "%s input spamming!", d->host );
		log_string( log_buf );
		write_to_descriptor( d->descriptor,
		    "\n\r*** PUT A LID ON IT!!! ***\n\r", 0 );
		strcpy( d->incomm, "quit" );
	    }
	}
    }

    /*
     * Do '!' substitution.
     */
    if ( d->incomm[0] == '!' )
	strcpy( d->incomm, d->inlast );
    else
	strcpy( d->inlast, d->incomm );

    /*
     * Shift the input buffer.
     */
    while ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
	i++;
    for ( j = 0; ( d->inbuf[j] = d->inbuf[i+j] ) != '\0'; j++ )
	;
    return;
}



/*
 * Low level output function.
 */
bool process_output( DESCRIPTOR_DATA *d, bool fPrompt )
{
    extern bool merc_down;

    /*
     * Bust a prompt.
     */
    if (d->pProtocol->WriteOOB)
        ;
    else if (fPrompt && !merc_down && d->connected == CON_PLAYING) {
        if ( d->showstr_point )
	    write_to_buffer( d,
  "[Please type (c)ontinue, (r)efresh, (b)ack, (h)elp, (q)uit, or RETURN]:  ",
			    0 );
	else
	{
	    CHAR_DATA *ch;

	    ch = d->original ? d->original : d->character;
	    if ( IS_SET(ch->act, PLR_BLANK) )
	        write_to_buffer( d, "\n\r", 2 );

	    if ( IS_SET(ch->act, PLR_PROMPT) )
	        bust_a_prompt( ch );

	    if ( IS_SET(ch->act, PLR_TELNET_GA) )
	        write_to_buffer( d, go_ahead_str, 0 );
	}
    }

    /*
     * Short-circuit if nothing to write.
     */
    if ( d->outtop == 0 )
	return TRUE;

    /*
     * Snoop-o-rama.
     */
    if ( d->snoop_by != NULL )
    {
	write_to_buffer( d->snoop_by, "% ", 2 );
	write_to_buffer( d->snoop_by, d->outbuf, d->outtop );
    }

    /*
     * OS-dependent output.
     */
    if ( !write_to_descriptor( d->descriptor, d->outbuf, d->outtop ) )
    {
	d->outtop = 0;
	return FALSE;
    }
    else
    {
	d->outtop = 0;
	return TRUE;
    }
}

/*
 * Bust a prompt (player settable prompt)
 * coded by Morgenes for Aldara Mud
 */
void bust_a_prompt( CHAR_DATA *ch )
{
   char buf[MAX_STRING_LENGTH];
   char buf2[MAX_STRING_LENGTH];
   const char *str;
   const char *i;
   char *point;

   if( ch->prompt == NULL || ch->prompt[0] == '\0' )
   {
      send_to_char( "\n\r\n\r", ch );
      return;
   }

   point = buf;
   str = ch->prompt;
   while( *str != '\0' )
   {
      if( *str != '%' )
      {
         *point++ = *str++;
         continue;
      }
      ++str;
      switch( *str )
      {
         default :
            i = " "; break;
         case 'h' :
            sprintf( buf2, "%d", ch->hit );
            i = buf2; break;
         case 'H' :
            sprintf( buf2, "%d", ch->max_hit );
            i = buf2; break;
         case 'm' :
            sprintf( buf2, "%d", ch->mana );
            i = buf2; break;
         case 'M' :
            sprintf( buf2, "%d", ch->max_mana );
            i = buf2; break;
         case 'v' :
            sprintf( buf2, "%d", ch->move );
            i = buf2; break;
         case 'V' :
            sprintf( buf2, "%d", ch->max_move );
            i = buf2; break;
         case 'x' :
            sprintf( buf2, "%d", ch->exp );
            i = buf2; break;
         case 'g' :
            sprintf( buf2, "%d", ch->gold);
            i = buf2; break;
         case 'a' :
            if( ch->level < 5 )
               sprintf( buf2, "%d", ch->alignment );
            else
               sprintf( buf2, "%s", IS_GOOD(ch) ? "good" : IS_EVIL(ch) ? 
                "evil" : "neutral" );
            i = buf2; break;
         case 'r' :
            if( ch->in_room != NULL )
               sprintf( buf2, "%s", ch->in_room->name );
            else
               sprintf( buf2, " " );
            i = buf2; break;
         case 'R' :
            if( IS_IMMORTAL( ch ) && ch->in_room != NULL )
               sprintf( buf2, "%d", ch->in_room->vnum );
            else
               sprintf( buf2, " " );
            i = buf2; break;
         case 'z' :
            if( IS_IMMORTAL( ch ) && ch->in_room != NULL )
               sprintf( buf2, "%s", ch->in_room->area->name );
            else
               sprintf( buf2, " " );
            i = buf2; break;
         case '%' :
            sprintf( buf2, "%%" );
            i = buf2; break;
      } 
      ++str;
      while( (*point = *i) != '\0' )
         ++point, ++i;      
   }
   write_to_buffer( ch->desc, buf, point - buf );
   return;
}

/*
 * Append onto an output buffer.
 */
void write_to_buffer( DESCRIPTOR_DATA *d, const char *txt, int length )
{
    txt = ProtocolOutput( d, txt, &length );
    if ( d->pProtocol->WriteOOB > 0 )
        --d->pProtocol->WriteOOB;

    /*
     * Find length in case caller didn't.
     */
    if ( length <= 0 )
	length = strlen(txt);

    /*
     * Initial \n\r if needed.
     */
    if ( d->outtop == 0 && !d->fcommand && !d->pProtocol->WriteOOB )
    {
	d->outbuf[0]	= '\n';
	d->outbuf[1]	= '\r';
	d->outtop	= 2;
    }

    /*
     * Expand the buffer as needed.
     */
    while ( d->outtop + length >= d->outsize )
    {
	char *outbuf;

	outbuf      = alloc_mem( 2 * d->outsize );
	strncpy( outbuf, d->outbuf, d->outtop );
	free_mem( d->outbuf, d->outsize );
	d->outbuf   = outbuf;
	d->outsize *= 2;
    }

    /*
     * Copy.
     */
    strcpy( d->outbuf + d->outtop, txt );
    d->outtop += length;
    return;
}



/*
 * Lowest level output function.
 * Write a block of text to the file descriptor.
 * If this gives errors on very long blocks (like 'ofind all'),
 *   try lowering the max block size.
 */
bool write_to_descriptor( int desc, char *txt, int length )
{
    int iStart;
    int nWrite;
    int nBlock;

#if defined(macintosh) || defined(MSDOS)
    if ( desc == 0 )
	desc = 1;
#endif

    if ( length <= 0 )
	length = strlen(txt);

    for ( iStart = 0; iStart < length; iStart += nWrite )
    {
	nBlock = UMIN( length - iStart, 4096 );
	if ( ( nWrite = write( desc, txt + iStart, nBlock ) ) < 0 )
	    { perror( "Write_to_descriptor" ); return FALSE; }
    } 

    return TRUE;
}



/*
 * Deal with sockets that haven't logged in yet.
 */
void nanny( DESCRIPTOR_DATA *d, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *ch;
    NOTE_DATA *pnote;
    char *pwdnew;
    char *p;
    int iClass;
    int lines;
    int notes;
    bool fOld;

    while ( isspace(*argument) )
	argument++;

    ch = d->character;

    switch ( d->connected )
    {

    default:
	bug( "Nanny: bad d->connected %d.", d->connected );
	close_socket( d );
	return;

    case CON_GET_NAME:
	if ( argument[0] == '\0' )
	{
	    close_socket( d );
	    return;
	}

	argument[0] = UPPER(argument[0]);
	if ( !check_parse_name( argument ) )
	{
	    write_to_buffer( d, "Illegal name, try another.\n\rName: ", 0 );
	    return;
	}

	fOld = load_char_obj( d, argument );
	ch   = d->character;

	if ( IS_SET(ch->act, PLR_DENY) )
	{
	    sprintf( log_buf, "Denying access to %s@%s.", argument, d->host );
	    log_string( log_buf );
	    write_to_buffer( d, "You are denied access.\n\r", 0 );
	    close_socket( d );
	    return;
	}

	if ( check_reconnect( d, argument, FALSE ) )
	{
	    fOld = TRUE;
	}
	else
	{
	    if ( wizlock && !IS_HERO( ch ) && !ch->wizbit )
	    {
		write_to_buffer( d, "The game is wizlocked.\n\r", 0 );
		close_socket( d );
		return;
	    }
	}

	if ( fOld )
	{
	    /* Old player */
	    write_to_buffer( d, "Password: ", 0 );
	    ProtocolNoEcho(d, true);
	    d->connected = CON_GET_OLD_PASSWORD;
	    return;
	}
	else
	{
	    /* New player */
	    /* New characters with same name fix by Salem's Lot */
	    if ( check_playing( d, ch->name ) )
	        return;
	    sprintf( buf, "Did I get that right, %s (Y/N)? ", argument );
	    write_to_buffer( d, buf, 0 );
	    d->connected = CON_CONFIRM_NEW_NAME;
	    return;
	}
	break;

    case CON_GET_OLD_PASSWORD:
#if defined(unix) || defined(linux)
	write_to_buffer( d, "\n\r", 2 );
#endif

	if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
	{
	    write_to_buffer( d, "Wrong password.\n\r", 0 );
	    close_socket( d );
	    return;
	}

	ProtocolNoEcho(d, false);

	if ( check_reconnect( d, ch->name, TRUE ) )
	    return;

	if ( check_playing( d, ch->name ) )
	    return;
		    
	sprintf( log_buf, "%s@%s has connected.", ch->name, d->host );
	log_string( log_buf );
	lines = ch->pcdata->pagelen;
	ch->pcdata->pagelen = 20;
	if ( IS_HERO(ch) )
	    do_help( ch, "imotd" );
	do_help( ch, "motd" );
	ch->pcdata->pagelen = lines;
	d->connected = CON_READ_MOTD;
	break;

    case CON_CONFIRM_NEW_NAME:
	switch ( *argument )
	{
	case 'y': case 'Y':
            ProtocolNoEcho(d, true);
	    sprintf( buf, "New character.\n\rGive me a password for %s: ", ch->name);
	    write_to_buffer( d, buf, 0 );
	    d->connected = CON_GET_NEW_PASSWORD;
	    break;

	case 'n': case 'N':
	    write_to_buffer( d, "Ok, what IS it, then? ", 0 );
	    free_char( d->character );
	    d->character = NULL;
	    d->connected = CON_GET_NAME;
	    break;

	default:
	    write_to_buffer( d, "Please type Yes or No? ", 0 );
	    break;
	}
	break;

    case CON_GET_NEW_PASSWORD:
#if defined(unix) || defined(linux)
	write_to_buffer( d, "\n\r", 2 );
#endif

	if ( strlen(argument) < 5 )
	{
	    write_to_buffer( d,
		"Password must be at least five characters long.\n\rPassword: ",
		0 );
	    return;
	}

	pwdnew = crypt( argument, ch->name );
	for ( p = pwdnew; *p != '\0'; p++ )
	{
	    if ( *p == '~' )
	    {
		write_to_buffer( d,
		    "New password not acceptable, try again.\n\rPassword: ",
		    0 );
		return;
	    }
	}

	free_string( ch->pcdata->pwd );
	ch->pcdata->pwd	= str_dup( pwdnew );
	write_to_buffer( d, "Please retype password: ", 0 );
	d->connected = CON_CONFIRM_NEW_PASSWORD;
	break;

    case CON_CONFIRM_NEW_PASSWORD:
#if defined(unix) || defined(linux)
	write_to_buffer( d, "\n\r", 2 );
#endif

	if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
	{
	    write_to_buffer( d, "Passwords don't match.\n\rRetype password: ",
		0 );
	    d->connected = CON_GET_NEW_PASSWORD;
	    return;
	}

	ProtocolNoEcho( d, false );
	write_to_buffer( d, "What is your sex (M/F/N)? ", 0 );
	d->connected = CON_GET_NEW_SEX;
	break;

    case CON_GET_NEW_SEX:
	switch ( argument[0] )
	{
	case 'm': case 'M': ch->sex = SEX_MALE;    break;
	case 'f': case 'F': ch->sex = SEX_FEMALE;  break;
	case 'n': case 'N': ch->sex = SEX_NEUTRAL; break;
	default:
	    write_to_buffer( d, "That's not a sex.\n\rWhat IS your sex? ", 0 );
	    return;
	}

	strcpy( buf, "Select a class [" );
	for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
	{
	    if ( iClass > 0 )
		strcat( buf, " " );
	    strcat( buf, class_table[iClass].who_name );
	}
	strcat( buf, "]: " );
	write_to_buffer( d, buf, 0 );
	d->connected = CON_GET_NEW_CLASS;
	break;

    case CON_GET_NEW_CLASS:
	for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
	{
	    if ( !str_cmp( argument, class_table[iClass].who_name ) )
	    {
		ch->class = iClass;
		break;
	    }
	}

	if ( iClass == MAX_CLASS )
	{
	    write_to_buffer( d,
		"That's not a class.\n\rWhat IS your class? ", 0 );
	    return;
	}

	sprintf( log_buf, "%s@%s new player.", ch->name, d->host );
	log_string( log_buf );
	write_to_buffer( d, "\n\r", 2 );
	ch->pcdata->pagelen = 20;
	ch->prompt = "<%hhp %mm %vmv> ";
	do_help( ch, "motd" );
	d->connected = CON_READ_MOTD;
	break;

    case CON_READ_MOTD:
	ch->next	= char_list;
	char_list	= ch;
	d->connected	= CON_PLAYING;

	send_to_char(
    "\n\rWelcome to Merc Diku Mud.  May your visit here be ... Mercenary.\n\r",
	    ch );

	if ( ch->level == 0 )
	{
	    OBJ_DATA *obj;

	    switch ( class_table[ch->class].attr_prime )
	    {
	    case APPLY_STR: ch->pcdata->perm_str = 16; break;
	    case APPLY_INT: ch->pcdata->perm_int = 16; break;
	    case APPLY_WIS: ch->pcdata->perm_wis = 16; break;
	    case APPLY_DEX: ch->pcdata->perm_dex = 16; break;
	    case APPLY_CON: ch->pcdata->perm_con = 16; break;
	    }

	    ch->level	= 1;
	    ch->exp	= 1000;
	    ch->hit	= ch->max_hit;
	    ch->mana	= ch->max_mana;
	    ch->move	= ch->max_move;
	    sprintf( buf, "the %s",
		title_table [ch->class] [ch->level]
		[ch->sex == SEX_FEMALE ? 1 : 0] );
	    set_title( ch, buf );

	    obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_BANNER), 0 );
	    obj_to_char( obj, ch );
	    equip_char( ch, obj, WEAR_LIGHT );

	    obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_VEST), 0 );
	    obj_to_char( obj, ch );
	    equip_char( ch, obj, WEAR_BODY );

	    obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_SHIELD), 0 );
	    obj_to_char( obj, ch );
	    equip_char( ch, obj, WEAR_SHIELD );

	    obj = create_object( get_obj_index(class_table[ch->class].weapon),
		0 );
	    obj_to_char( obj, ch );
	    equip_char( ch, obj, WEAR_WIELD );

	    char_to_room( ch, get_room_index( ROOM_VNUM_SCHOOL ) );
	}
	else if ( ch->in_room != NULL )
	{
	    char_to_room( ch, ch->in_room );
	}
	else if ( IS_IMMORTAL(ch) )
	{
	    char_to_room( ch, get_room_index( ROOM_VNUM_CHAT ) );
	}
	else
	{
	    char_to_room( ch, get_room_index( ROOM_VNUM_TEMPLE ) );
	}

	act( "$n has entered the game.", ch, NULL, NULL, TO_ROOM );
	MXPSendTag(d, "<VERSION>");
	do_look(ch, "auto");
	/* check for new notes */
	notes = 0;

	for ( pnote = note_list; pnote != NULL; pnote = pnote->next )
	    if ( is_note_to( ch, pnote ) && str_cmp( ch->name, pnote->sender )
		&& pnote->date_stamp > ch->last_note )
	        notes++;

	if ( notes == 1 )
	    send_to_char( "\n\rYou have one new note waiting.\n\r", ch );
	else
	    if ( notes > 1 )
	    {
		sprintf( buf, "\n\rYou have %d new notes waiting.\n\r",
			notes );
		send_to_char( buf, ch );
	    }

	break;
    }

    return;
}



/*
 * Parse a name for acceptability.
 */
bool check_parse_name( char *name )
{
    /*
     * Reserved words.
     */
    if ( is_name( name, "all auto immortal self someone" ) )
	return FALSE;

    /*
     * Length restrictions.
     */
    if ( strlen(name) <  3 )
	return FALSE;

#if defined(MSDOS)
    if ( strlen(name) >  8 )
	return FALSE;
#endif

#if defined(macintosh) || defined(unix) || defined(linux)
    if ( strlen(name) > 12 )
	return FALSE;
#endif

    /*
     * Alphanumerics only.
     * Lock out IllIll twits.
     */
    {
	char *pc;
	bool fIll;

	fIll = TRUE;
	for ( pc = name; *pc != '\0'; pc++ )
	{
	    if ( !isalpha(*pc) )
		return FALSE;
	    if ( LOWER(*pc) != 'i' && LOWER(*pc) != 'l' )
		fIll = FALSE;
	}

	if ( fIll )
	    return FALSE;
    }

    /*
     * Prevent players from naming themselves after mobs.
     */
    {
	extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
	MOB_INDEX_DATA *pMobIndex;
	int iHash;

	for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
	{
	    for ( pMobIndex  = mob_index_hash[iHash];
		  pMobIndex != NULL;
		  pMobIndex  = pMobIndex->next )
	    {
		if ( is_name( name, pMobIndex->player_name ) )
		    return FALSE;
	    }
	}
    }

    return TRUE;
}



/*
 * Look for link-dead player to reconnect.
 */
bool check_reconnect( DESCRIPTOR_DATA *d, char *name, bool fConn )
{
    CHAR_DATA *ch;
    OBJ_DATA *obj;

    for ( ch = char_list; ch != NULL; ch = ch->next )
    {
	if ( !IS_NPC(ch)
	&& ( !fConn || ch->desc == NULL )
	&&   !str_cmp( d->character->name, ch->name ) )
	{
	    if ( fConn == FALSE )
	    {
		free_string( d->character->pcdata->pwd );
		d->character->pcdata->pwd = str_dup( ch->pcdata->pwd );
	    }
	    else
	    {
		free_char( d->character );
		d->character = ch;
		ch->desc	 = d;
		ch->timer	 = 0;
		send_to_char( "Reconnecting.\n\r", ch );
		act( "$n has reconnected.", ch, NULL, NULL, TO_ROOM );
		sprintf( log_buf, "%s@%s reconnected.", ch->name, d->host );
		log_string( log_buf );
		d->connected = CON_PLAYING;
		MXPSendTag(d, "<VERSION>");

		/*
		 * Contributed by Gene Choi
		 */
		if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL
		    && obj->item_type == ITEM_LIGHT
		    && obj->value[2] != 0
		    && ch->in_room != NULL )
		    ++ch->in_room->light;
	    }
	    return TRUE;
	}
    }

    return FALSE;
}



/*
 * Check if already playing.
 */
bool check_playing( DESCRIPTOR_DATA *d, char *name )
{
    DESCRIPTOR_DATA *dold;

    for ( dold = descriptor_list; dold; dold = dold->next )
    {
	if ( dold != d
	&&   dold->character != NULL
	&&   dold->connected != CON_GET_NAME
	&&   dold->connected != CON_GET_OLD_PASSWORD
	&&   !str_cmp( name, dold->original
	         ? dold->original->name : dold->character->name ) )
	{
	    write_to_buffer( d, "Already playing.\n\rName: ", 0 );
	    d->connected = CON_GET_NAME;
	    if ( d->character != NULL )
	    {
		free_char( d->character );
		d->character = NULL;
	    }
	    return TRUE;
	}
    }

    return FALSE;
}



void stop_idling( CHAR_DATA *ch )
{
    if ( ch == NULL
    ||   ch->desc == NULL
    ||   ch->desc->connected != CON_PLAYING
    ||   ch->was_in_room == NULL 
    ||   ch->in_room != get_room_index( ROOM_VNUM_LIMBO ) )
	return;

    ch->timer = 0;
    char_from_room( ch );
    char_to_room( ch, ch->was_in_room );
    ch->was_in_room	= NULL;
    act( "$n has returned from the void.", ch, NULL, NULL, TO_ROOM );
    return;
}


/*
 * Write to one char.
 */
void send_to_char( const char *txt, CHAR_DATA *ch )
{
    if ( txt == NULL || ch->desc == NULL )
        return;
    ch->desc->showstr_head = alloc_mem( strlen( txt ) + 1 );
    strcpy( ch->desc->showstr_head, txt );
    ch->desc->showstr_point = ch->desc->showstr_head;
    show_string( ch->desc, "" );

}

/* The heart of the pager.  Thanks to N'Atas-Ha, ThePrincedom
   for porting this SillyMud code for MERC 2.0 and laying down the groundwork.
   Thanks to Blackstar, hopper.cs.uiowa.edu 4000 for which
   the improvements to the pager was modeled from.  - Kahn */

void show_string(struct descriptor_data *d, char *input)
{
  char buffer[ MAX_STRING_LENGTH ];
  char buf[ MAX_INPUT_LENGTH ];
  register char *scan, *chk;
  int lines = 0, toggle=1;

  one_argument(input, buf);

  switch( UPPER( buf[0] ) )
  {
  case '\0':
  case 'C': /* show next page of text */
    lines = 0;
    break;

  case 'R': /* refresh current page of text */
    lines = - 1 - (d->character->pcdata->pagelen);
    break;

  case 'B': /* scroll back a page of text */
    lines = -(2*d->character->pcdata->pagelen);
    break;

  case 'H': /* Show some help */
    write_to_buffer( d,
        "C, or Return = continue, R = redraw this page,\n\r", 0 );
    write_to_buffer( d,
        "B = back one page, H = this help, Q or other keys = exit.\n\r\n\r",
		    0 );
    lines = - 1 - (d->character->pcdata->pagelen);
    break;

  default: /*otherwise, stop the text viewing */
    if ( d->showstr_head )
    {
      free_string( d->showstr_head );
      d->showstr_head  = 0;
    }
    d->showstr_point = 0;
    return;

  }

  /* do any backing up necessary */
  if (lines < 0)
  {
    for ( scan = d->showstr_point; scan > d->showstr_head; scan-- )
         if ( ( *scan == '\n' ) || ( *scan == '\r' ) )
	 {
	     toggle = -toggle;
	     if ( toggle < 0 )
	         if ( !( ++lines ) )
		     break;
	 }
    d->showstr_point = scan;
  }

  /* show a chunk */
  lines  = 0;
  toggle = 1;
  for ( scan = buffer; ; scan++, d->showstr_point++ )
       if ( ( ( *scan = *d->showstr_point ) == '\n' || *scan == '\r' )
	   && ( toggle = -toggle ) < 0 )
	   lines++;
       else
	   if ( !*scan || ( d->character && !IS_NPC( d->character )
			  && lines >= d->character->pcdata->pagelen) )
	   {

	       *scan = '\0';
	       write_to_buffer( d, buffer, strlen( buffer ) );

	     /* See if this is the end (or near the end) of the string */
	       for ( chk = d->showstr_point; isspace( *chk ); chk++ );
	       if ( !*chk )
	       {
		   if ( d->showstr_head )
		   {
		      free_string( d->showstr_head );
		      d->showstr_head  = 0;
		   }
		   d->showstr_point = 0;
	       }
	       return;
	   }

  return;
}

/*
 * The primary output interface for formatted output.
 */
void act( const char *format, CHAR_DATA *ch, const void *arg1,
	 const void *arg2, int type )
{
    static char * const he_she	[] = { "it",  "he",  "she" };
    static char * const him_her	[] = { "it",  "him", "her" };
    static char * const his_her	[] = { "its", "his", "her" };

    char buf[MAX_STRING_LENGTH];
    char fname[MAX_INPUT_LENGTH];
    CHAR_DATA *to;
    CHAR_DATA *vch = (CHAR_DATA *) arg2;
    OBJ_DATA *obj1 = (OBJ_DATA  *) arg1;
    OBJ_DATA *obj2 = (OBJ_DATA  *) arg2;
    const char *str;
    const char *i;
    char *point;

    /*
     * Discard null and zero-length messages.
     */
    if ( format == NULL || format[0] == '\0' )
	return;

    to = ch->in_room->people;
    if ( type == TO_VICT )
    {
	if ( vch == NULL )
	{
	    bug( "Act: null vch with TO_VICT.", 0 );
	    return;
	}
	to = vch->in_room->people;
    }

    for ( ; to != NULL; to = to->next_in_room )
    {
	if ((to->desc == NULL && IS_NPC(to)) || !IS_AWAKE(to))
	    continue;

	if ( type == TO_CHAR && to != ch )
	    continue;
	if ( type == TO_VICT && ( to != vch || to == ch ) )
	    continue;
	if ( type == TO_ROOM && to == ch )
	    continue;
	if ( type == TO_NOTVICT && (to == ch || to == vch) )
	    continue;

	point	= buf;
	str	= format;
	while ( *str != '\0' )
	{
	    if ( *str != '$' )
	    {
		*point++ = *str++;
		continue;
	    }
	    ++str;

	    if ( arg2 == NULL && *str >= 'A' && *str <= 'Z' )
	    {
		bug( "Act: missing arg2 for code %d.", *str );
		i = " <@@@> ";
	    }
	    else
	    {
		switch ( *str )
		{
		default:  bug( "Act: bad code %d.", *str );
			  i = " <@@@> ";				break;
		/* Thx alex for 't' idea */
		case 't': i = (char *) arg1;				break;
		case 'T': i = (char *) arg2;          			break;
		case 'n': i = PERS( ch,  to  );				break;
		case 'N': i = PERS( vch, to  );				break;
		case 'e': i = he_she  [URANGE(0, ch  ->sex, 2)];	break;
		case 'E': i = he_she  [URANGE(0, vch ->sex, 2)];	break;
		case 'm': i = him_her [URANGE(0, ch  ->sex, 2)];	break;
		case 'M': i = him_her [URANGE(0, vch ->sex, 2)];	break;
		case 's': i = his_her [URANGE(0, ch  ->sex, 2)];	break;
		case 'S': i = his_her [URANGE(0, vch ->sex, 2)];	break;

		case 'p':
		    i = can_see_obj( to, obj1 )
			    ? obj1->short_descr
			    : "something";
		    break;

		case 'P':
		    i = can_see_obj( to, obj2 )
			    ? obj2->short_descr
			    : "something";
		    break;

		case 'd':
		    if ( arg2 == NULL || ((char *) arg2)[0] == '\0' )
		    {
			i = "door";
		    }
		    else
		    {
			one_argument( (char *) arg2, fname );
			i = fname;
		    }
		    break;
		}
	    }

	    ++str;
	    while ( ( *point = *i ) != '\0' )
		++point, ++i;
	}

	*point++ = '\n';
	*point++ = '\r';
	buf[0]   = UPPER(buf[0]);
	if (to->desc)
	  write_to_buffer( to->desc, buf, point - buf );
    }

    return;
}



/*
 * Macintosh support functions.
 */
#if defined(macintosh)
int gettimeofday( struct timeval *tp, void *tzp )
{
    tp->tv_sec  = time( NULL );
    tp->tv_usec = 0;
}
#endif
