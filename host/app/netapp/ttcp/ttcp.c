
#include "lwip/opt.h"
#include <stdarg.h>
#include <lwip/sockets.h>
#include <lwip/netdb.h>
#include <netapp/getopt.h>
#include <os_wrapper.h>
#include <log.h>
#include <rtos.h>

/*
 *	T T C P . C
 *
 * Test TCP connection.  Makes a connection on port 5001
 * and transfers fabricated buffers or data copied from stdin.
 *
 * Usable on 4.2, 4.3, and 4.1a systems by defining one of
 * BSD42 BSD43 (BSD41a)
 * Machines using System V with BSD sockets should define SYSV.
 *
 * Modified for operation under 4.2BSD, 18 Dec 84
 *      T.C. Slattery, USNA
 * Minor improvements, Mike Muuss and Terry Slattery, 16-Oct-85.
 * Modified in 1989 at Silicon Graphics, Inc.
 *	catch SIGPIPE to be able to print stats when receiver has died 
 *	for tcp, don't look for sentinel during reads to allow small transfers
 *	increased default buffer size to 8K, g_nNumBuffersToSend to 2K to transfer 16MB
 *	moved default port to 5001, beyond IPPORT_USERRESERVED
 *	make sinkmode default because it is more popular, 
 *		-s now means don't sink/source.
 *       in sink/source mode use pattern generator to fill send
 *       buffer (once at init time). received data is tossed.
 *	count number of read/write system calls to see effects of 
 *		blocking from full socket buffers
 *	for tcp, -D option turns off buffered writes (sets TCP_NODELAY sockopt)
 *	buffer alignment options, -A and -O
 *	print stats in a format that's a bit easier to use with grep & awk
 *	for SYSV, mimic BSD routines to use most of the existing timing code
 * Modified by Steve Miller of the University of Maryland, College Park
 *	-b sets the socket buffer size (SO_SNDBUF/SO_RCVBUF)
 * Modified Sept. 1989 at Silicon Graphics, Inc.
 *	restored -s sense at request of tcs@brl
 * Modified Oct. 1991 at Silicon Graphics, Inc.
 *	use getopt(3) for option processing, add -f and -T options.
 *	SGI IRIX 3.3 and 4.0 releases don't need #define SYSV.
 *
 * PCAUSA Version 1.00.00.01 - Modified April, 1999 at Printing
 * Communications Assoc., Inc. (PCAUSA) PCAUSA. Initial port to Winsock.
 *
 * PCAUSA Version 1.00.00.02 - Modified January, 2000 at Printing
 * Communications Assoc., Inc. (PCAUSA) to fix setting of setsockopt call
 * for TCP_NODELAY.
 *
 * PCAUSA Version 2.01.01.04 - Modified May, 2002 at Printing
 * Communications Assoc., Inc. (PCAUSA). Incorporates substantial rework
 * suit the author's style and purpose (as a sockets programming learning
 * program and a TCP/IP exploration tool).
 *
 * Although the functionality has been extended, the application is
 * backwards-compatible with previous PCAUSA releases.
 *
 * PCAUSA Version 2.01.01.05 - Modified May, 2002 at PCAUSA. Modification
 * to allow SO_RVCBUF and SO_SNDBUF values of zero(0) to be handled.
 *
 * PCAUSA Version 2.01.01.06 - Modified April, 2003 at PCAUSA. Fixed minor
 * bug in TTCP transmitter. PCAUSA PCATTCP preamble was not inserted
 * correctly when building transmit buffer.
 * 
 * PCAUSA Version 2.01.01.07 - Modified November, 2003 at PCAUSA.
 * Incorporated fix identified by Clarkson University that reduces
 * hangs when ending the UDP transmitter test. See additional comments
 * in TTCP_TransmitUDP module.
 * 
 * PCAUSA Version 2.01.01.08 - Build now under Microsoft Visual Studio .NET
 * 2003.
 * 
 * Distribution Status -
 *      Public Domain.  Distribution Unlimited.
 */

#define  PCATTCP_VERSION   "2.01.01.08"

#define INVALID_SOCKET                  -1
#define SOCKET_ERROR                    -1


/////////////////////////////////////////////////////////////////////////////
//// Type definitions
//
typedef bool                    BOOLEAN;
typedef char *                  PCHAR;
typedef char			        CHAR;
typedef u8			            UCHAR;
typedef u16                     USHORT;
typedef s32                     SOCKET;
typedef u32                     DWORD;
typedef struct sockaddr_in      SOCKADDR_IN;
typedef struct sockaddr *    PSOCKADDR;
//typedef struct sockaddr_in *    PSOCKADDR;






#if 0
struct hostent {
    char  *h_name;      /* Official name of the host. */
    char **h_aliases;   /* A pointer to an array of pointers to alternative host names,
                           terminated by a null pointer. */
    int    h_addrtype;  /* Address type. */
    int    h_length;    /* The length, in bytes, of the address. */
    char **h_addr_list; /* A pointer to an array of pointers to network addresses (in
                           network byte order) for the host, terminated by a null pointer. */
//#define h_addr h_addr_list[0] /* for backward compatibility */
};
#endif


/////////////////////////////////////////////////////////////////////////////
//// GLOBAL VARIABLES
//


#ifndef IPPORT_TTCP
#define IPPORT_TTCP          5001
#endif

typedef
struct _CMD_BLOCK
{
   //
   // Test Command/Configuration Data
   //
   BOOLEAN     m_bTransmit;
   USHORT      m_Protocol;

   SOCKADDR_IN m_RemoteAddr;        // Host Byte Order
   int         m_nRemoteAddrLen;

   SOCKADDR_IN m_LocalAddress;      // Host Byte Order

   USHORT      m_Port;              // Host Byte Order

   BOOLEAN     m_bOptContinuous;
   int         m_nOptWriteDelay;    // milliseconds of delay before each write

   BOOLEAN     m_bOptMultiReceiver;

   BOOLEAN     m_bTouchRecvData;

   BOOLEAN     m_bSinkMode;         // FALSE = normal I/O, TRUE = sink/source mode

   int         m_nNumBuffersToSend; // number of buffers to send in sinkmode

   int         m_nBufferSize;       // length of buffer
   int         m_nBufOffset;        // align buffer to this
   int         m_nBufAlign;         //modulo this

   BOOLEAN     m_bUseSockOptBufSize;
   int         m_nSockOptBufSize;   // socket buffer size to use

   int         m_nSockOptNoDelay;   // set TCP_NODELAY socket option

   BOOLEAN     m_bSockOptDebug;     // TRUE = set SO_DEBUG socket option

   SOCKET      m_ExtraSocket;
   
}  CMD_BLOCK, *PCMD_BLOCK;


typedef
struct _TEST_BLOCK
{
   //
   // Command/Configuration Data
   //
   CMD_BLOCK   m_Cmd;

   //
   // Instance Data
   //
   PCHAR       m_pBufBase;          // ptr to dynamic buffer (base)
   PCHAR       m_pBuf;              // ptr to dynamic buffer (aligned)

   SOCKET      m_Socket_fd;         // fd of network send/receive socket

   //
   // Statistics
   //
   DWORD          m_tStart;
   DWORD          m_tFinish;

   unsigned long  m_numCalls;		   // # of I/O system calls
   double         m_nbytes;         // bytes on net
}
   TEST_BLOCK, *PTEST_BLOCK;

int b_flag = 0;      // use mread()
int one = 1;         // for 4.3 BSD style setsockopt()
int zero = 0;        // for 4.3 BSD style setsockopt()

int verbose = 0;     // 0=print basic info, 1=print cpu rate, proc
				         // resource usage.

char fmt = 'K';      // output format:
                     // k = kilobits, K = kilobytes,
                     // m = megabits, M = megabytes, 
                     // g = gigabits, G = gigabytes

#define  UDP_GUARD_BUFFER_LENGTH  4

//
// Forward Procedure Prototypes
//
void TTCP_ExitTest( PTEST_BLOCK pTBlk, BOOLEAN bExitProcess );
void TTCP_ExitTestOnWSAError( PTEST_BLOCK pTBlk, PCHAR pFnc );
void TTCP_ExitTestOnCRTError( PTEST_BLOCK pTBlk, PCHAR pFnc );

int TTCP_Nread( PTEST_BLOCK pTBlk, int count );
int TTCP_Nwrite( PTEST_BLOCK pTBlk, int count );
int TTCP_mread( PTEST_BLOCK pTBlk, unsigned n );

char *outfmt(double b);

void delay( int us );

/////////////////////////////////////////////////////////////////////////////
//// Miscellaneous Support Routines
//
#ifdef __SSV_UNIX_SIM__

#include <ssv_ex_lib.h>
#include   <sys/time.h>
struct timeval start,stop,diff;

#endif
extern int l3_errno; 



int WSAGetLastError(void)
{
    return l3_errno;
}


void TTCP_LogMsg( const char *format, ... )
{

    //   FILE *logfd = NULL;
    //   FILE *logfd = stderr;

    char  szBuffer[ 256 ];
    va_list  marker;

    //   if( !logfd )
    //   {
    //      LogOpen();
    //   }

    //   if( !logfd )
    //   {
    //      return;
    //   }

    va_start( marker, format );

    vsprintf( szBuffer, format, marker );

    //   fprintf( logfd, "%s", szBuffer );
    LOG_PRINTF("%s", szBuffer);
}










void TTCP_InitStatistics( PTEST_BLOCK pTBlk )
{
   pTBlk->m_numCalls = 0;		// # of I/O system calls
   pTBlk->m_nbytes = 0;      // bytes on net

#ifdef __SSV_UNIX_SIM__
   gettimeofday(&start,0);
#else
   pTBlk->m_tStart = sys_now();
#endif
}


void TTCP_LogStatistics( PTEST_BLOCK pTBlk )
{
   double realt;		         // user, real time (seconds)

#ifdef __SSV_UNIX_SIM__
   gettimeofday(&stop,0);   
#else
   pTBlk->m_tFinish = sys_now();
#endif

#ifdef __SSV_UNIX_SIM__
	//timeval_subtract(&diff,&start,&stop); 
	realt = ((stop.tv_sec*1000+stop.tv_usec)-(start.tv_sec*1000+start.tv_usec))/1000;  //diff.tv_usec/1000+diff.tv_sec;
#else
   	realt = ((double )pTBlk->m_tFinish - (double )pTBlk->m_tStart)/1000;
#endif



   if( pTBlk->m_Cmd.m_Protocol == IPPROTO_UDP )
   {
      if( pTBlk->m_Cmd.m_bTransmit )
      {
         TTCP_LogMsg( "  Statistics  : UDP -> %s:%d\r\n",
            inet_ntoa( pTBlk->m_Cmd.m_RemoteAddr.sin_addr ),
            pTBlk->m_Cmd.m_Port );
      }
      else
      {
         TTCP_LogMsg( "  Statistics  : UDP <- %s:%d\r\n",
            inet_ntoa( pTBlk->m_Cmd.m_RemoteAddr.sin_addr ),
            htons( pTBlk->m_Cmd.m_RemoteAddr.sin_port ) );
      }
   }
   else
   {
      if( pTBlk->m_Cmd.m_bTransmit )
      {
         TTCP_LogMsg( "  Statistics  : TCP -> %s:%d\r\n",
            inet_ntoa( pTBlk->m_Cmd.m_RemoteAddr.sin_addr ),
            pTBlk->m_Cmd.m_Port
            );
      }
      else
      {
         TTCP_LogMsg( "  Statistics  : TCP <- %s:%d\r\n",
            inet_ntoa( pTBlk->m_Cmd.m_RemoteAddr.sin_addr ),
            htons( pTBlk->m_Cmd.m_RemoteAddr.sin_port ) );
      }
   }

   TTCP_LogMsg( 
      "%.0f bytes in %.2f real seconds = %s/sec +++\r\n",
      pTBlk->m_nbytes,
      realt,
      outfmt(pTBlk->m_nbytes/realt)
      );

   TTCP_LogMsg( "numCalls: %d; msec/call: %.2f; calls/sec: %.2f\r\n",
      pTBlk->m_numCalls,
      1024.0 * realt/((double )pTBlk->m_numCalls),
      ((double )pTBlk->m_numCalls)/realt
      );
}

/////////////////////////////////////////////////////////////////////////////
//// TEST_BLOCK Support Routines
//
// Remarks
// A TEST_BLOCK structure is allocated for each TTCP primary test routine:
//
//    TTCP_TransmitTCP
//    TTCP_TransmitUDP
//    TTCP_ReceiveTCP
//    TTCP_ListenTCP
//    TTCP_ReceiveUDP
//
// The TEST_BLOCK is a private "instance data" structure that contains
// all of the information necessary to perform the function. It includes
// a copy of the CMD_BLOCK parameters that control the test's operation
// as well as other members to keep track of SOCKETs, buffers and
// statistics associated with the test function.
//
// Each TTCP primary test function allocates its own TEST_BLOCK and is
// responsible for eventually freeing it.
//
// In the blocking single-threaded PCATTCP version use of this mechanism
// is certainly overkill, and serves no truly useful purpose. Global
// data would have been just as effective.
//
// HOWEVER, this approach was taken in anticipation of eventually enhancing
// the test to be multi-threaded - which will actually be trivial.
//

PTEST_BLOCK
TTCP_AllocateTestBlock( PCMD_BLOCK pCmdBlk )
{
   PTEST_BLOCK pTBlk = NULL;

   pTBlk = (PTEST_BLOCK )MALLOC( sizeof( TEST_BLOCK ) );

   if( !pTBlk )
   {
      TTCP_ExitTestOnCRTError( NULL, "malloc" );

      return( NULL );
   }

   MEMSET((void *)pTBlk, 0x00, sizeof( TEST_BLOCK ) );

   MEMCPY( &pTBlk->m_Cmd, pCmdBlk, sizeof( CMD_BLOCK ) );

   pTBlk->m_Socket_fd = INVALID_SOCKET; // fd of network send/receive socket

   return( pTBlk );
}


void TTCP_FreeTestBlock( PTEST_BLOCK pTBlk )
{
   if( !pTBlk )
   {
      return;
   }

   if( pTBlk->m_pBufBase )
   {
      FREE( pTBlk->m_pBufBase );
   }

   pTBlk->m_pBufBase = NULL;

   OS_MsDelay(1000);

   if( pTBlk->m_Socket_fd != INVALID_SOCKET )
   {
      closesocket( pTBlk->m_Socket_fd );

      pTBlk->m_Socket_fd = INVALID_SOCKET;
   }

   pTBlk->m_Socket_fd = INVALID_SOCKET;

   FREE( pTBlk );
}


/////////////////////////////////////////////////////////////////////////////
//// TTCP Exit Routines
//
// Remarks
// The TTCP tests exit by calling one of 
//
//  TTCP_ExitTest           - Call for normal exit.
//  TTCP_ExitTestOnWSAError - Call to exit when Winsock error is encountered.
//  TTCP_ExitTestOnCRTError - Call to exit when OS/DOS error is encountered.
//
// The exit routines systematically free the TEST_BLOCK resources and the
// TEST_BLOCK structure itself.
//
// They exit by calling ExitThread or ExitProcess. Understand that in
// the single-threaded version a call to ExitThread simply exits the
// main program thread. That is: it is equivalent to calling ExitProcess.
//

void TTCP_ExitTest( PTEST_BLOCK pTBlk, BOOLEAN bExitProcess )
{
   if( pTBlk )
   {
      TTCP_FreeTestBlock( pTBlk );
   }

#if 0
   if( bExitProcess )
   {
      WSACleanup();

      ExitProcess( 1 );
   }

   ExitThread( 1 );
#endif   
}

void TTCP_ExitTestOnWSAError( PTEST_BLOCK pTBlk, PCHAR pFnc )
{
   int nError = WSAGetLastError();

   TTCP_LogMsg( "*** Winsock Error: %s Failed; Error: %d (0x%8.8X)\r\n",
      pFnc, nError, nError );

   TTCP_ExitTest( pTBlk, TRUE );
}

void TTCP_ExitTestOnCRTError( PTEST_BLOCK pTBlk, PCHAR pFnc )
{
   TTCP_LogMsg( "*** CRT Error: %s Failed; Error: %d (0x%8.8X)\r\n",
      pFnc, l3_errno, l3_errno );

   TTCP_ExitTest( pTBlk, TRUE );
}

void TTCP_SetConfigDefaults( PCMD_BLOCK pCmdBlk )
{
   MEMSET((void *) pCmdBlk, 0x00, sizeof( CMD_BLOCK ) );

   pCmdBlk->m_bTransmit = FALSE;
   pCmdBlk->m_Protocol = IPPROTO_TCP;

   MEMSET((void *)
      &pCmdBlk->m_RemoteAddr,
      0x00,
      sizeof( SOCKADDR_IN )
      );

   pCmdBlk->m_Port = IPPORT_TTCP;

   pCmdBlk->m_bOptContinuous = FALSE;
   pCmdBlk->m_nOptWriteDelay = 0;

   pCmdBlk->m_bOptMultiReceiver = FALSE;

   pCmdBlk->m_bTouchRecvData = FALSE;

   pCmdBlk->m_bSinkMode = TRUE;     // FALSE = normal I/O, TRUE = sink/source mode

   //
   // Setup SinkMode Default
   // ----------------------
   // SinkMode description:
   //   TRUE  -> A pattern generator is used fill the send buffer. This
   //            is done only once. Received data is simply counted.
   //   FALSE -> Data to be sent is read from stdin. Received data is
   //            written to stdout.
   //
   // g_nNumBuffersToSend specifies the number of buffers to be sent
   // in SinkMode.
   //
   pCmdBlk->m_bSinkMode = TRUE;     // FALSE = normal I/O, TRUE = sink/source mode
   pCmdBlk->m_nNumBuffersToSend = 2 * 1024; // number of buffers to send in sinkmode

   //
   // Setup Size Of Send/Receive Buffer
   //
//   pCmdBlk->m_pBufBase = NULL;
//   pCmdBlk->m_pBuf = NULL;
   pCmdBlk->m_nBufferSize = 8 * 1024;

   pCmdBlk->m_nBufOffset = 0;             // align buffer to this
   pCmdBlk->m_nBufAlign = 16*1024;        // modulo this

   pCmdBlk->m_bUseSockOptBufSize = FALSE; // socket buffer size to use
   pCmdBlk->m_nSockOptBufSize = 0;        // socket buffer size to use

   pCmdBlk->m_nSockOptNoDelay = 1;        // set TCP_NODELAY socket option

   pCmdBlk->m_bSockOptDebug = FALSE;      // TRUE = set SO_DEBUG socket option
}

// Fill Buffer With Printable Characters...
void TTCP_FillPattern( PTEST_BLOCK pTBlk )
{
   register char c;
   UCHAR PBPreamble[] = "PCAUSA PCATTCP Pattern";   // 22 Bytes
   int cnt = pTBlk->m_Cmd.m_nBufferSize;
   char  *cp = pTBlk->m_pBuf;

   c = 0;

   //
   // Insert "PCAUSA Pattern" Preamble
   //
   if( cnt > 22 )
   {
      MEMCPY( cp, PBPreamble, 22 );
      cnt -= 22;
	  cp += 22;
   }

   while( cnt-- > 0 )
   {
      while( !isprint((c&0x7F)) )
      {
         c++;
      }

      *cp++ = (c++&0x7F);
   }
}

BOOLEAN TTCP_AllocateBuffer( PTEST_BLOCK pTBlk )
{
   //
   // Setup Buffer Configuration
   //
   if( (pTBlk->m_pBufBase = (PCHAR )MALLOC(
               pTBlk->m_Cmd.m_nBufferSize + pTBlk->m_Cmd.m_nBufAlign)) == (PCHAR )NULL
      )
   {
      return( FALSE );  // Failed
   }

   //
   // Align The Buffer
   //
   pTBlk->m_pBuf = pTBlk->m_pBufBase;

   if (pTBlk->m_Cmd.m_nBufAlign != 0)
      pTBlk->m_pBuf += (
         pTBlk->m_Cmd.m_nBufAlign
         - ((int)pTBlk->m_pBuf % pTBlk->m_Cmd.m_nBufAlign
         ) + pTBlk->m_Cmd.m_nBufOffset) % pTBlk->m_Cmd.m_nBufAlign;

   TTCP_LogMsg( "  Buffer Size : %d; Alignment: %d/%d\r\n",
      pTBlk->m_Cmd.m_nBufferSize,
      pTBlk->m_Cmd.m_nBufAlign,
      pTBlk->m_Cmd.m_nBufOffset
      );

   return( TRUE );  // Success
}


/////////////////////////////////////////////////////////////////////////////
//// Primary TTCP Functions
//

/////////////////////////////////////////////////////////////////////////////
//// TTCP_TransmitTCP
//
// Purpose
// TTCP TCP transmitter.
//
// Parameters
//   pCmdBlk -         Pointer to CMD_BLOCK that contains options and other
//                     configuration information to be used for the transmit
//                     test.
//   pRemoteHostName - Pointer to null-terminated name of remote host.
//                     May be IP address or DNS-resolvable name.
//
// Return Value
// Returns zero(0) for normal return. Non-zero otherwise.
//
// Remarks
//

DWORD TTCP_TransmitTCP( PCMD_BLOCK pCmdBlk, PCHAR pRemoteHostName )
{
   PTEST_BLOCK pTBlk = NULL;
   u32_t   optlen;

   //
   // Say Hello
   //
   TTCP_LogMsg( "%s\r\n",__func__);

   //
   // Allocate Test Instance Data
   // ---------------------------
   // Allocate a TEST_BLOCK for this specific instance. The TEST_BLOCK
   // contains a copy of the calleer's CMD_BLOCK as well as additional
   // members that are used to perform this test.
   //
   pTBlk = TTCP_AllocateTestBlock( pCmdBlk );

   if( !pTBlk )
   {
      TTCP_ExitTestOnCRTError( NULL, "malloc" );
      return( 1 );
   }

   //
   // Setup Remote IP Addresses For Test
   //
   if( atoi( pRemoteHostName ) > 0 )
   {
      //
      // Numeric
      //
      pTBlk->m_Cmd.m_RemoteAddr.sin_family = AF_INET;
		pTBlk->m_Cmd.m_RemoteAddr.sin_addr.s_addr = inet_addr( pRemoteHostName );
   }
   else
   {
#if LWIP_DNS
      struct hostent *addr;
	  unsigned long addr_tmp;

      TTCP_LogMsg( "  Remote Host : %s\r\n", pRemoteHostName );

      if ((addr=gethostbyname( pRemoteHostName )) == NULL)
      {
         TTCP_ExitTestOnWSAError( pTBlk, "gethostbyname" );
         return (1);
      }

      pTBlk->m_Cmd.m_RemoteAddr.sin_family = addr->h_addrtype;

      MEMCPY((char*)&addr_tmp, addr->h_addr, addr->h_length );
      pTBlk->m_Cmd.m_RemoteAddr.sin_addr.s_addr = addr_tmp;
#else
    TTCP_ExitTestOnWSAError( pTBlk, "Unknown remote host" );
    return (1);
#endif
   }
   
   pTBlk->m_Cmd.m_RemoteAddr.sin_port = htons( pTBlk->m_Cmd.m_Port );

   //
   // Setup Local IP Addresses For Test
   //
   pTBlk->m_Cmd.m_LocalAddress.sin_family = AF_INET;
   pTBlk->m_Cmd.m_LocalAddress.sin_port = 0;		/* free choice */

   TTCP_LogMsg( "  Transmit    : TCP -> %s:%d\r\n",
      inet_ntoa( pTBlk->m_Cmd.m_RemoteAddr.sin_addr ),
      pTBlk->m_Cmd.m_Port
      );

   //
   // Allocate The Buffer Send/Receive Data
   //
   if( !TTCP_AllocateBuffer( pTBlk ) )
   {
      TTCP_ExitTestOnCRTError( pTBlk, "malloc" );
   }

   //
   // Open Socket For Test
   //
   if( ( pTBlk->m_Socket_fd = socket( AF_INET, SOCK_STREAM, 0 ) )
      == INVALID_SOCKET
      )
   {
      TTCP_ExitTestOnWSAError( pTBlk, "socket" );
      return (1);
   }

   //
   // Bind Socket With Local Address
   //
   if( bind(
         pTBlk->m_Socket_fd,
         (PSOCKADDR )&pTBlk->m_Cmd.m_LocalAddress,
         sizeof(pTBlk->m_Cmd.m_LocalAddress)
         ) == SOCKET_ERROR
      )
   {
      TTCP_ExitTestOnWSAError( pTBlk, "bind" );
      return (1);
   }

   if( pTBlk->m_Cmd.m_bUseSockOptBufSize )
   {
      if( setsockopt(
            pTBlk->m_Socket_fd,
            SOL_SOCKET,
            SO_SNDBUF,
            (char * )&pTBlk->m_Cmd.m_nSockOptBufSize,
            sizeof pTBlk->m_Cmd.m_nSockOptBufSize
            ) == SOCKET_ERROR
         )
      {
         TTCP_ExitTestOnWSAError( pTBlk, "setsockopt: SO_SNDBUF" );
         return (1);
      }

      TTCP_LogMsg( "  SO_SNDBUF   : %d\r\n", pTBlk->m_Cmd.m_nSockOptBufSize );
   }

   //
   // Start TCP Connections
   //
   optlen = sizeof( pTBlk->m_Cmd.m_nSockOptNoDelay );

   //
   // We are the client if transmitting
   //
   if( pTBlk->m_Cmd.m_bSockOptDebug )
   {
      if( setsockopt(
            pTBlk->m_Socket_fd,
            SOL_SOCKET,
            SO_DEBUG,
            (PCHAR )&one,     // Boolean
            sizeof(one)
            ) == SOCKET_ERROR
         )
      {
         TTCP_ExitTestOnWSAError( pTBlk, "setsockopt: SO_DEBUG" );
         return (1);
      }
   }

   //
   // Set TCP_NODELAY Send Option
   //
   if( pTBlk->m_Cmd.m_nSockOptNoDelay )
   {
      if( setsockopt(
            pTBlk->m_Socket_fd,
            IPPROTO_TCP,
            TCP_NODELAY, 
            (PCHAR )&one,     // Boolean
            sizeof(one)
            ) == SOCKET_ERROR
         )
      {
         int nError = WSAGetLastError();
         TTCP_LogMsg( "  Error: 0x%8.8X\r\n", nError );
         TTCP_LogMsg("setsockopt: TCP_NODELAY option failed,%d\r\n",__LINE__);
      }
   }

   //
   // Query And Display TCP_NODELAY Send Option
   //
   if( getsockopt(
         pTBlk->m_Socket_fd,
         IPPROTO_TCP,
         TCP_NODELAY, 
         (PCHAR )&pTBlk->m_Cmd.m_nSockOptNoDelay,
            &optlen
         ) != SOCKET_ERROR
      )
   {
      if( pTBlk->m_Cmd.m_nSockOptNoDelay )
      {
         TTCP_LogMsg( "  TCP_NODELAY : ENABLED (%d)\r\n",
            pTBlk->m_Cmd.m_nSockOptNoDelay );
      }
      else
      {
         TTCP_LogMsg( "  TCP_NODELAY : DISABLED (%d)\r\n",
            pTBlk->m_Cmd.m_nSockOptNoDelay );
      }
   }
   else
   {
      int nError = WSAGetLastError();
      TTCP_LogMsg( "  Error: 0x%8.8X\r\n", nError );
      TTCP_LogMsg("getsockopt: TCP_NODELAY option failed,%d\r\n",__LINE__);
   }

   //
   // Connect To Remote Server
   //
   if(connect( pTBlk->m_Socket_fd, (PSOCKADDR )&pTBlk->m_Cmd.m_RemoteAddr,
      sizeof(pTBlk->m_Cmd.m_RemoteAddr) ) == SOCKET_ERROR)
   {
      TTCP_ExitTestOnWSAError( pTBlk, "connect" );
      return (1);
   }

   TTCP_LogMsg( "  Connect     : Connected to %s:%d\r\n",
      inet_ntoa( pTBlk->m_Cmd.m_RemoteAddr.sin_addr ), pTBlk->m_Cmd.m_Port );

   TTCP_InitStatistics( pTBlk );

//  errno = 0;

   if( pTBlk->m_Cmd.m_nOptWriteDelay )
   {
      TTCP_LogMsg( "  Write Delay : %d milliseconds\r\n",
         pTBlk->m_Cmd.m_nOptWriteDelay
         );
   }

   if( pTBlk->m_Cmd.m_bSinkMode )
   {      
      TTCP_FillPattern( pTBlk );

      if( pTBlk->m_Cmd.m_bOptContinuous )
      {
         TTCP_LogMsg( "  Send Mode   : Sending Pattern CONTINUOUS\r\n");

         while( TTCP_Nwrite( pTBlk, pTBlk->m_Cmd.m_nBufferSize) == pTBlk->m_Cmd.m_nBufferSize)
         {
            pTBlk->m_nbytes += pTBlk->m_Cmd.m_nBufferSize;
         }
      }
      else
      {
         TTCP_LogMsg( "  Send Mode   : Send Pattern; Number of Buffers: %d\n",
            pTBlk->m_Cmd.m_nNumBuffersToSend
            );

         while (pTBlk->m_Cmd.m_nNumBuffersToSend--
            && TTCP_Nwrite( pTBlk, pTBlk->m_Cmd.m_nBufferSize) == pTBlk->m_Cmd.m_nBufferSize)
         {
            pTBlk->m_nbytes += pTBlk->m_Cmd.m_nBufferSize;
         }
      }
   }
   else
   {
      register int cnt;

      TTCP_LogMsg( "  Send Mode   : Read from STDIN\r\n");

      //
      // Read From stdin And Write To Remote
      //
      while( ( cnt = read(0, pTBlk->m_pBuf,pTBlk->m_Cmd.m_nBufferSize ) ) > 0
         && TTCP_Nwrite( pTBlk, cnt) == cnt
         )
      {
         pTBlk->m_nbytes += cnt;
      }
   }

   TTCP_LogMsg( "XMIT TCP Errno %d\r\n", l3_errno);
   //if(errno)
   //   TTCP_ExitTestOnCRTError( pTBlk, "IO" );

   TTCP_LogStatistics( pTBlk );

   TTCP_FreeTestBlock( pTBlk );

   return( 0 );
}


/////////////////////////////////////////////////////////////////////////////
//// TTCP_TransmitUDP
//
// Purpose
// TTCP UDP transmitter.
//
// Parameters
//   pCmdBlk -         Pointer to CMD_BLOCK that contains options and other
//                     configuration information to be used for the transmit
//                     test.
//   pRemoteHostName - Pointer to null-terminated name of remote host.
//                     May be IP address or DNS-resolvable name.
//
// Return Value
// Returns zero(0) for normal return. Non-zero otherwise.
//
// Remarks
//

DWORD TTCP_TransmitUDP( PCMD_BLOCK pCmdBlk, PCHAR pRemoteHostName )
{
   PTEST_BLOCK pTBlk = NULL;

   //
   // Say Hello
   //
   TTCP_LogMsg( "UDP Transmit Test\r\n");

   //
   // Allocate Test Instance Data
   // ---------------------------
   // Allocate a TEST_BLOCK for this specific instance. The TEST_BLOCK
   // contains a copy of the calleer's CMD_BLOCK as well as additional
   // members that are used to perform this test.
   //
   pTBlk = TTCP_AllocateTestBlock( pCmdBlk );

   if( !pTBlk )
   {
      TTCP_ExitTestOnCRTError( NULL, "malloc" );

      return( 1 );
   }

   //
   // Setup Remote IP Addresses For Test
   //
   if( atoi( pRemoteHostName ) > 0 )
   {
      //
      // Numeric
      //
      pTBlk->m_Cmd.m_RemoteAddr.sin_family = AF_INET;
		pTBlk->m_Cmd.m_RemoteAddr.sin_addr.s_addr = inet_addr( pRemoteHostName );
   }
   else
   {
   
#if LWIP_DNS
      struct hostent *addr;
	   unsigned long addr_tmp;

      TTCP_LogMsg( "  Remote Host : %s\r\n", pRemoteHostName );

      if ((addr=gethostbyname( pRemoteHostName )) == NULL)
      {
         TTCP_ExitTestOnWSAError( pTBlk, "gethostbyname" );
         return (1);
      }

      pTBlk->m_Cmd.m_RemoteAddr.sin_family = addr->h_addrtype;

      MEMCPY((void*)&addr_tmp, (void*)addr->h_addr, addr->h_length );
      pTBlk->m_Cmd.m_RemoteAddr.sin_addr.s_addr = addr_tmp;
#else
      TTCP_ExitTestOnWSAError( pTBlk, "Unknown remote host" );
      return (1);
#endif
   }
   
   pTBlk->m_Cmd.m_RemoteAddr.sin_port = htons( pTBlk->m_Cmd.m_Port );

   //
   // Setup Local IP Addresses For Test
   //
   pTBlk->m_Cmd.m_LocalAddress.sin_family = AF_INET;
   pTBlk->m_Cmd.m_LocalAddress.sin_port = 0;		/* free choice */

   TTCP_LogMsg( "  Transmit    : UDP -> %s:%d\r\n",
      inet_ntoa( pTBlk->m_Cmd.m_RemoteAddr.sin_addr ),
      pTBlk->m_Cmd.m_Port );

   //
   // Setup Buffer Configuration
   //
   if( pTBlk->m_Cmd.m_nBufferSize <= UDP_GUARD_BUFFER_LENGTH )
   {
      pTBlk->m_Cmd.m_nBufferSize = UDP_GUARD_BUFFER_LENGTH + 1; // send more than the sentinel size
   }

   //
   // Allocate The Buffer Send/Receive Data
   //
   if( !TTCP_AllocateBuffer( pTBlk ) )
   {
      TTCP_ExitTestOnCRTError( pTBlk, "malloc" );
   }

   //
   // Open Socket For Test
   //
   if( (pTBlk->m_Socket_fd = socket( AF_INET, SOCK_DGRAM, 0 ) ) == INVALID_SOCKET
      )
   {
      TTCP_ExitTestOnWSAError( pTBlk, "socket" );
      return (1);
   }

   //
   // Bind Socket With Local Address
   //
   if( bind(
         pTBlk->m_Socket_fd,
         (PSOCKADDR )&pTBlk->m_Cmd.m_LocalAddress,
         sizeof(pTBlk->m_Cmd.m_LocalAddress)
         ) == SOCKET_ERROR
      )
   {
      TTCP_ExitTestOnWSAError( pTBlk, "bind" );
      return (1);
   }

   if( pTBlk->m_Cmd.m_bUseSockOptBufSize )
   {
      if( setsockopt(
            pTBlk->m_Socket_fd,
            SOL_SOCKET,
            SO_SNDBUF,
            (char * )&pTBlk->m_Cmd.m_nSockOptBufSize,
            sizeof pTBlk->m_Cmd.m_nSockOptBufSize
            ) == SOCKET_ERROR
         )
      {
         TTCP_ExitTestOnWSAError( pTBlk, "setsockopt: SO_SNDBUF" );
         return (1);
      }

      TTCP_LogMsg( "  SO_SNDBUF   : %d\r\n", pTBlk->m_Cmd.m_nSockOptBufSize );
   }

   TTCP_InitStatistics( pTBlk );
//  errno = 0;

   if( pTBlk->m_Cmd.m_nOptWriteDelay )
   {
      TTCP_LogMsg( "  Write Delay : %d milliseconds\r\n",
         pTBlk->m_Cmd.m_nOptWriteDelay
         );
   }

   if( pTBlk->m_Cmd.m_bSinkMode )
   {      
      TTCP_FillPattern( pTBlk );

      TTCP_Nwrite( pTBlk, UDP_GUARD_BUFFER_LENGTH ); /* rcvr start */

      if( pTBlk->m_Cmd.m_bOptContinuous )
      {
         TTCP_LogMsg( "  Send Mode   : Send Pattern CONTINUOUS\r\n");

         while (TTCP_Nwrite( pTBlk, pTBlk->m_Cmd.m_nBufferSize) == pTBlk->m_Cmd.m_nBufferSize)
            pTBlk->m_nbytes += pTBlk->m_Cmd.m_nBufferSize;
      }
      else
      {
         TTCP_LogMsg( "  Send Mode   : Send Pattern; Number of Buffers: %d\r\n",
            pTBlk->m_Cmd.m_nNumBuffersToSend
            );

         while (pTBlk->m_Cmd.m_nNumBuffersToSend--
               && TTCP_Nwrite( pTBlk, pTBlk->m_Cmd.m_nBufferSize) == pTBlk->m_Cmd.m_nBufferSize)
            pTBlk->m_nbytes += pTBlk->m_Cmd.m_nBufferSize;
      }

      TTCP_Nwrite( pTBlk, UDP_GUARD_BUFFER_LENGTH ); /* rcvr end */
   }
   else
   {
      register int cnt;

      TTCP_LogMsg( "  Send Mode   : Read from STDIN\r\n");

      //
      // Read From stdin And Write To Remote
      //
      while( ( cnt = read(0, pTBlk->m_pBuf,pTBlk->m_Cmd.m_nBufferSize ) ) > 0
         && TTCP_Nwrite( pTBlk, cnt) == cnt
         )
      {
         pTBlk->m_nbytes += cnt;
      }
   }

	if(l3_errno)
      TTCP_ExitTestOnCRTError( pTBlk, "IO" );

   TTCP_LogStatistics( pTBlk );

   //
   // Pause To Allow Receiver To Flush Receive Buffers
   // ------------------------------------------------
   // PCATTCP Version 2.01.01.06 and prior (as well as the original TTCP
   // application)were subject to failure at various points. This was due
   // to the fact that UDP has no flow control, so a UDP transfer overwhelms
   // the receiving computer. The reason it was failing was that the end marker
   // of the data transfer was getting lost in the network while the receiving
   // computer was being overwhelmed with data. This would cause the receiving
   // computer to hang, waiting for more data, but the transmitting computer
   // had already finished sending all the data along with the end markers.
   //
   // The folks at Clarkson University identified a simple fix for this
   // weakness which PCAUSA has adopted in V2.01.01.07. See the URL:
   //
   // http://www.clarkson.edu/projects/itl/HOWTOS/mcnair_summer2002/Paper/User_Manual_PCATTCP_Controller_1.00.htm
   //
   // A simple pause by the transmitter before the end markers were sent would
   // allow the receiving computer time to free its internal receiving buffers
   // and obtain an end marker.
   //
   OS_MsDelay( 1000 );

   TTCP_Nwrite( pTBlk, UDP_GUARD_BUFFER_LENGTH );   // rcvr end
   TTCP_Nwrite( pTBlk, UDP_GUARD_BUFFER_LENGTH );   // rcvr end
   TTCP_Nwrite( pTBlk, UDP_GUARD_BUFFER_LENGTH );   // rcvr end
   TTCP_Nwrite( pTBlk, UDP_GUARD_BUFFER_LENGTH );   // rcvr end

   TTCP_FreeTestBlock( pTBlk );

   return( 0 );
}


/////////////////////////////////////////////////////////////////////////////
//// TTCP_ReceiveTCP
//
// Purpose
// TTCP TCP stream receiver.
//
// Parameters
//   pCmdBlk - Pointer to CMD_BLOCK that contains options and other
//             configuration information to be used to receive on
//             the stream.
//   conn_fd - The connection socket, which returned from a call to
//             accept which is made in TTCP_ListenTCP.
//
// Return Value
// Returns zero(0) for normal return. Non-zero otherwise.
//
// Remarks
// The TTCP_ListenTCP routine listens for connections on the TTCP port.
// When a connection is accepted in TTCP_ListenTCP it then calls this
// function to perform the TCP receive test on the connection socket.
//
// Ownership of the conn_fd socket is passed to this routine, which is
// responsible for eventually closing it.
//

DWORD TTCP_ReceiveTCP( PCMD_BLOCK pCmdBlk )
{
   PTEST_BLOCK pTBlk = NULL;

   //
   // Allocate Test Instance Data
   // ---------------------------
   // Allocate a TEST_BLOCK for this specific instance. The TEST_BLOCK
   // contains a copy of the calleer's CMD_BLOCK as well as additional
   // members that are used to perform this test.
   //
   pTBlk = TTCP_AllocateTestBlock( pCmdBlk );

   if( !pTBlk )
   {
      closesocket( pCmdBlk->m_ExtraSocket );

      TTCP_ExitTestOnCRTError( NULL, "malloc" );

      return( 1 );
   }

   //
   // Save The Connection Socket
   //
   pTBlk->m_Socket_fd = pCmdBlk->m_ExtraSocket;

   pTBlk->m_Cmd.m_Protocol = IPPROTO_TCP;

   //
   // Allocate The Buffer Send/Receive Data
   //
   if( !TTCP_AllocateBuffer( pTBlk ) )
   {
      TTCP_ExitTestOnCRTError( pTBlk, "malloc" );

      return( 1 );
   }

   TTCP_InitStatistics( pTBlk );
   l3_errno = 0;

   if( pTBlk->m_Cmd.m_bSinkMode )
   {      
      register int cnt;

      //
      // Discard Received Data
      //
      TTCP_LogMsg( "  Receive Mode: Sinking (discarding) Data\r\n");

      while( (cnt=TTCP_Nread( pTBlk, pTBlk->m_Cmd.m_nBufferSize) ) > 0)
      {
         pTBlk->m_nbytes += cnt;
         //TTCP_LogMsg( "R:%d", pTBlk->m_nbytes);
      }
   }
   else
   {
      register int cnt;

      //
      // Read From Remote And Write To stdout
      //
      TTCP_LogMsg( "  Receive Mode: Writing Received Data To STDOUT\r\n");

      while( ( cnt = TTCP_Nread( pTBlk, pTBlk->m_Cmd.m_nBufferSize ) ) > 0
         && write(1, pTBlk->m_pBuf,cnt) == cnt
         )
      {
         pTBlk->m_nbytes += cnt;
      }
   }

   TTCP_LogMsg( "Recv TCP Errno %d\r\n", l3_errno);
	//if(errno)
    //  TTCP_ExitTestOnCRTError( pTBlk, "IO" );

   TTCP_LogStatistics( pTBlk );

   TTCP_FreeTestBlock( pTBlk );

   return( 0 );
}


/////////////////////////////////////////////////////////////////////////////
//// TTCP_ListenTCP
//
// Purpose
// TTCP TCP connection listener.
//
// Parameters
//   pCmdBlk - Pointer to CMD_BLOCK that contains options and other
//             configuration information to be used to listen for
//             connections.
//
// Return Value
// Returns zero(0) for normal return. Non-zero otherwise.
//
// Remarks
// The TTCP_ListenTCP routine listens for connections on the TTCP port.
//
// When a connection is accepted TTCP_ListenTCP calls TTCP_ReceiveTCP
// function to perform the actual TCP receive test.
//
// Ownership of the conn_fd socket is passed to TTCP_ReceiveTCP routine,
// which is responsible for eventually closing conn_fd.
//

DWORD TTCP_ListenTCP( PCMD_BLOCK pCmdBlk )
{
   PTEST_BLOCK    pTBlk = NULL;
   SOCKET         conn_fd;
   BOOLEAN        bDoAccept = TRUE;
//   CHAR           szLocalHostName[ 128 ];

   TTCP_LogMsg( "TCP Receive Test\r\n");

   //
   // Allocate Test Instance Data
   // ---------------------------
   // Allocate a TEST_BLOCK for this specific instance. The TEST_BLOCK
   // contains a copy of the calleer's CMD_BLOCK as well as additional
   // members that are used to perform this test.
   //
   pTBlk = TTCP_AllocateTestBlock( pCmdBlk );

   if( !pTBlk )
   {
      TTCP_ExitTestOnCRTError( NULL, "malloc" );

      return( 1 );
   }

//   sprintf( szLocalHostName, "Unknown" );
//   gethostname( szLocalHostName, 128 );
//   TTCP_LogMsg( "  Local Host  : %s\n", szLocalHostName );

   //
   // Setup Local IP Addresses For Test
   //
   pTBlk->m_Cmd.m_LocalAddress.sin_family = AF_INET;
   pTBlk->m_Cmd.m_LocalAddress.sin_port =  htons( pTBlk->m_Cmd.m_Port );

   //
   // Open Listening Socket
   //
   if( (pTBlk->m_Socket_fd = socket( AF_INET, SOCK_STREAM, 0 ) ) == INVALID_SOCKET )
   {
      TTCP_ExitTestOnWSAError( pTBlk, "socket" );
      return (1);
   }

   //
   // Bind Socket With Local Address
   //
   if( bind(
         pTBlk->m_Socket_fd,
         (PSOCKADDR )&pTBlk->m_Cmd.m_LocalAddress,
         sizeof(pTBlk->m_Cmd.m_LocalAddress)
         ) == SOCKET_ERROR
      )
   {
      TTCP_ExitTestOnWSAError( pTBlk, "bind" );
      return (1);
   }

   if( pTBlk->m_Cmd.m_bUseSockOptBufSize )
   {
      if( setsockopt(
            pTBlk->m_Socket_fd,
            SOL_SOCKET,
            SO_RCVBUF,
            (char * )&pTBlk->m_Cmd.m_nSockOptBufSize,
            sizeof pTBlk->m_Cmd.m_nSockOptBufSize
            ) == SOCKET_ERROR
         )
      {
         TTCP_ExitTestOnWSAError( pTBlk, "setsockopt: SO_RCVBUF" );
         return (1);
      }

      TTCP_LogMsg( "  SO_RCVBUF   : %d\r\n", pTBlk->m_Cmd.m_nSockOptBufSize );
   }

   //
   // Start TCP Connections
   // ---------------------
   // We Are The Server.
   //
   listen( pTBlk->m_Socket_fd, 0 );  // allow a queue of 0

   if( pTBlk->m_Cmd.m_bSockOptDebug )
   {
      if( setsockopt(
            pTBlk->m_Socket_fd,
            SOL_SOCKET,
            SO_DEBUG,
            (PCHAR )&one,        // Boolean
            sizeof(one)
            ) == SOCKET_ERROR
         )
      {
         TTCP_ExitTestOnWSAError( pTBlk, "setsockopt: SO_DEBUG" );
         return (1);
      }
   }
   
   pTBlk->m_Cmd.m_nRemoteAddrLen = sizeof( pTBlk->m_Cmd.m_RemoteAddr );

   while( bDoAccept )
   {
//      DWORD    tid;     // Thread ID

      TTCP_LogMsg( "**************\r\n");
      TTCP_LogMsg( "  Listening...: On port %d\r\n", pTBlk->m_Cmd.m_Port );

      if( ( conn_fd = accept( pTBlk->m_Socket_fd, (PSOCKADDR )&pTBlk->m_Cmd.m_RemoteAddr,
         (u32_t*)&pTBlk->m_Cmd.m_nRemoteAddrLen) ) == SOCKET_ERROR)
      {
         TTCP_ExitTestOnWSAError( pTBlk, "accept" );
         return (1);
      }
      else
      {
         SOCKADDR_IN peer;
         int peerlen = sizeof(peer);

         if (getpeername( conn_fd, (PSOCKADDR ) &peer, 
			   (u32_t*)&peerlen) == SOCKET_ERROR)
         {
            TTCP_ExitTestOnWSAError( pTBlk, "getpeername" );
            return (1);
         }

         pCmdBlk->m_RemoteAddr.sin_addr.s_addr = peer.sin_addr.s_addr;
         pCmdBlk->m_RemoteAddr.sin_port = peer.sin_port;

         TTCP_LogMsg( "\r\n  Accept      : TCP <- %s:%d\r\n",
            inet_ntoa( pCmdBlk->m_RemoteAddr.sin_addr ),
            htons( pCmdBlk->m_RemoteAddr.sin_port ) );
      }

      pTBlk->m_Cmd.m_ExtraSocket = conn_fd;

#if 0
      if( pCmdBlk->m_bOptMultiReceiver )
      {
         CreateThread(
            NULL,
            0,
            (LPTHREAD_START_ROUTINE )TTCP_ReceiveTCP,
            &pTBlk->m_Cmd,
            0,
            &tid
            );
      }      
      else
#endif        
      {
         TTCP_ReceiveTCP( &pTBlk->m_Cmd );
      }

      if( !pTBlk->m_Cmd.m_bOptContinuous )
      {
         bDoAccept = FALSE;
      }
   }

   TTCP_FreeTestBlock( pTBlk );

   return( 0 );
}

/////////////////////////////////////////////////////////////////////////////
//// TTCP_ReceiveUDP
//
// Purpose
// TTCP UDP datagram receiver.
//
// Parameters
//   pCmdBlk - Pointer to CMD_BLOCK that contains options and other
//             configuration information.
//
// Return Value
// Returns zero(0) for normal return. Non-zero otherwise.
//
// Remarks
// The TTCP_ReceiveUDP routine performs the TTCP UDP receive test.
//

DWORD TTCP_ReceiveUDP( PCMD_BLOCK pCmdBlk )
{
   PTEST_BLOCK pTBlk = NULL;
   BOOLEAN     bContinue = TRUE;

   TTCP_LogMsg( "UDP Receive Test\r\n");

   //
   // Allocate Test Instance Data
   // ---------------------------
   // Allocate a TEST_BLOCK for this specific instance. The TEST_BLOCK
   // contains a copy of the calleer's CMD_BLOCK as well as additional
   // members that are used to perform this test.
   //
   pTBlk = TTCP_AllocateTestBlock( pCmdBlk );

   if( !pTBlk )
   {
      TTCP_ExitTestOnCRTError( NULL, "malloc" );

      return( 1 );
   }

   TTCP_LogMsg( "  Protocol   : UDP\r\n");
   TTCP_LogMsg( "  Port       : %d\r\n", pTBlk->m_Cmd.m_Port );

   //
   // Setup Local IP Addresses For Test
   //
   pTBlk->m_Cmd.m_LocalAddress.sin_family = AF_INET;
   pTBlk->m_Cmd.m_LocalAddress.sin_port =  htons( pTBlk->m_Cmd.m_Port );

   //
   // Setup Buffer Configuration
   //
   if( pTBlk->m_Cmd.m_nBufferSize <= UDP_GUARD_BUFFER_LENGTH
      )
   {
      pTBlk->m_Cmd.m_nBufferSize = UDP_GUARD_BUFFER_LENGTH + 1; // send more than the sentinel size
   }

   //
   // Allocate The Buffer Send/Receive Data
   //
   if( !TTCP_AllocateBuffer( pTBlk ) )
   {
      TTCP_ExitTestOnCRTError( pTBlk, "malloc" );
   }

   //
   // Open Socket For Test
   //
   if( (pTBlk->m_Socket_fd = socket( AF_INET, SOCK_DGRAM, 0 ) ) == INVALID_SOCKET )
   {
      TTCP_ExitTestOnWSAError( pTBlk, "socket" );
      return (1);
   }

   //
   // Bind Socket With Local Address
   //
   if( bind(
         pTBlk->m_Socket_fd,
         (PSOCKADDR )&pTBlk->m_Cmd.m_LocalAddress,
         sizeof(pTBlk->m_Cmd.m_LocalAddress)
         ) == SOCKET_ERROR
      )
   {
      TTCP_ExitTestOnWSAError( pTBlk, "bind" );
      return (1);
   }

   if( pTBlk->m_Cmd.m_bUseSockOptBufSize )
   {
      if( setsockopt(
            pTBlk->m_Socket_fd,
            SOL_SOCKET,
            SO_RCVBUF,
            (char * )&pTBlk->m_Cmd.m_nSockOptBufSize,
            sizeof pTBlk->m_Cmd.m_nSockOptBufSize
            ) == SOCKET_ERROR
         )
      {
         TTCP_ExitTestOnWSAError( pTBlk, "setsockopt: SO_RCVBUF" );
         return (1);
      }

      TTCP_LogMsg( "  SO_RCVBUF   : %d\r\n", pTBlk->m_Cmd.m_nSockOptBufSize );
   }

   while( bContinue )
   {
      TTCP_InitStatistics( pTBlk );
      l3_errno = 0;

      MEMSET((void *)
         &pTBlk->m_Cmd.m_RemoteAddr,
         0x00,
         sizeof( SOCKADDR_IN )
         );

      if( pTBlk->m_Cmd.m_bSinkMode )
      {      
         register int cnt;
         int going = 0;
         BOOLEAN bIsFirst = TRUE;

         while( (cnt=TTCP_Nread( pTBlk, pTBlk->m_Cmd.m_nBufferSize)) > 0 )
         {
            if( cnt <= UDP_GUARD_BUFFER_LENGTH )
            {
               if( going )
               {
                  going = 0;
                  break;	/* "EOF" */
               }

               going = 1;
               TTCP_InitStatistics( pTBlk );
            }
            else
            {
               if( bIsFirst )
               {
                  TTCP_LogMsg( "  recvfrom    : UDP <- %s:%d\r\n",
                     inet_ntoa( pTBlk->m_Cmd.m_RemoteAddr.sin_addr ),
                     htons( pTBlk->m_Cmd.m_RemoteAddr.sin_port ) );

                  bIsFirst = FALSE;
               }

               pTBlk->m_nbytes += cnt;
            }
         }
      }
      else
      {
         register int cnt;

         //
         // Read From Remote And Write To stdout
         //
         while( ( cnt = TTCP_Nread( pTBlk, pTBlk->m_Cmd.m_nBufferSize ) ) > 0
            && write(1, pTBlk->m_pBuf,cnt) == cnt
            )
         {
            pTBlk->m_nbytes += cnt;
         }
      }

	   if(l3_errno)
         TTCP_ExitTestOnCRTError( pTBlk, "IO" );

      if( pTBlk->m_nbytes > 0 )
      {
         TTCP_LogStatistics( pTBlk );
      }

      if( !pTBlk->m_Cmd.m_bOptContinuous )
      {
         bContinue = FALSE;
      }
   }

   TTCP_FreeTestBlock( pTBlk );

   return( 0 );
}

//
// Usage Message
//
void ttcp_print_usage()
{
    PRINTF("Usage: ttcp -t [-options] host [ < in ]\r\n");
    PRINTF("       ttcp -r [-options > out]\r\n");
    PRINTF("Common options:\r\n");
    PRINTF("   -l ##  length of bufs read from or written to network (default 8192)\r\n");
    PRINTF("   -u     use UDP instead of TCP\r\n");
    PRINTF("   -p ##  port number to send to or listen at (default 5001)\r\n");
    PRINTF("   -s     toggle sinkmode (enabled by default)\r\n");
    PRINTF("            sinkmode enabled:\r\n");
    PRINTF("               -t: source (transmit) fabricated pattern\r\n");
    PRINTF("               -r: sink (discard) all received data\r\n");
    PRINTF("            sinkmode disabled:\r\n");
    PRINTF("               -t: reads data to be transmitted from stdin\r\n");
    PRINTF("               -r: writes received data to stdout\r\n");
    PRINTF("   -A     align the start of buffers to this modulus (default 16384)\r\n");
    PRINTF("   -O     start buffers at this offset from the modulus (default 0)\r\n");
    PRINTF("   -v     verbose: print more statistics\r\n");
    PRINTF("   -d     set SO_DEBUG socket option\r\n");
    PRINTF("   -b ##  set socket buffer size (if supported)\r\n");
    PRINTF("   -f X   format for rate: k,K = kilo{bit,byte}; m,M = mega; g,G = giga\r\n");
    PRINTF("   -c       -t: send continuously\r\n");
    PRINTF("            -r: accept multiple connections sequentially\r\n");
    PRINTF("   -R     concurrent TCP/UDP multithreaded receiver\r\n");
    PRINTF("Options specific to -t:\r\n");
    PRINTF("   -n ##  number of source bufs written to network (default 2048)\r\n");
    PRINTF("   -D     don't buffer TCP writes (sets TCP_NODELAY socket option)\r\n");
    PRINTF("   -w ##  milliseconds of delay before each write (default 0)\r\n");
    PRINTF("Options specific to -r:\r\n");
    PRINTF("   -B     for -s, only output full blocks as specified by -l (for TAR)\r\n");
    PRINTF("   -T     \"touch\": access each byte as it's read\r\n");
}

/////////////////////////////////////////////////////////////////////////////
//// TTCP_ParseCommandLine
//
// Purpose
// Parse the console application command-line arguments and setup the
// CMD_BLOCK/
//
// Parameters
//   pCmdBlk - Pointer to CMD_BLOCK to be filled with parsed option
//             information.
//
// Return Value
// Returns zero(0) for normal return. Non-zero otherwise.
//
// Remarks
//

int TTCP_ParseCommandLine( PCMD_BLOCK pCmdBlk, int argc, char **argv )
{
   int   c;

	if (argc < 2)
   {
#ifdef __SSV_UNIX_SIM__
      ttcp_print_usage();
#endif
      return( !0 );
   }

   while (optind != argc)
   {
	   c = getopt(argc, argv, "cdrstuvBDRTb:f:l:n:p:A:O:w:" );

		switch (c)
      {
         case EOF:
            optarg = argv[optind];
            optind++;
            break;

         case 'B':
            b_flag = 1;
            break;

         case 't':
            pCmdBlk->m_bTransmit = TRUE;
            break;

         case 'r':
            pCmdBlk->m_bTransmit = FALSE;
            break;

         case 'c':
            pCmdBlk->m_bOptContinuous = TRUE;
            break;

         case 'd':
            pCmdBlk->m_bSockOptDebug = 1;
            break;

         case 'D':
            pCmdBlk->m_nSockOptNoDelay = 1;
            break;

         case 'R':
            pCmdBlk->m_bOptContinuous = TRUE;
            pCmdBlk->m_bTransmit = FALSE;
            pCmdBlk->m_bOptMultiReceiver = TRUE;
            break;

         case 'n':
            pCmdBlk->m_nNumBuffersToSend = atoi(optarg);
            break;

         case 'l':
            pCmdBlk->m_nBufferSize = atoi(optarg);
            break;

         case 's':
            pCmdBlk->m_bSinkMode = !pCmdBlk->m_bSinkMode;
            break;

         case 'p':
            pCmdBlk->m_Port = atoi(optarg);
            break;

         case 'u':
            pCmdBlk->m_Protocol = IPPROTO_UDP;
            break;

         case 'v':
            verbose = 1;
            break;

         case 'A':
            pCmdBlk->m_nBufAlign = atoi(optarg);
            break;

         case 'O':
            pCmdBlk->m_nBufOffset = atoi(optarg);
            break;

         case 'b':
            pCmdBlk->m_bUseSockOptBufSize = TRUE;
            pCmdBlk->m_nSockOptBufSize = atoi(optarg);
            if( pCmdBlk->m_nSockOptBufSize < 0 )
            {
               pCmdBlk->m_nSockOptBufSize = 0;
            }
            break;

         case 'f':
            fmt = *optarg;
            break;

         case 'T':
            pCmdBlk->m_bTouchRecvData = 1;
            break;

         case 'w':
            pCmdBlk->m_nOptWriteDelay = atoi(optarg);
            break;

		   default:
#ifdef __SSV_UNIX_SIM__
            ttcp_print_usage();
#endif
            resetopt();
            return( !0 );
      }
   }


   resetopt();
   return( 0 );
}


/////////////////////////////////////////////////////////////////////////////
//// MAIN Program Entry Point
//

/////////////////////////////////////////////////////////////////////////////
//// main
//
// Purpose
// Console application main program entry point.
//
// Parameters
//   pCmdBlk - Pointer to CMD_BLOCK to be filled with parsed option
//             information.
//
// Return Value
// Returns zero(0) for normal return. Non-zero otherwise.
//
// Remarks
//

int net_app_ttcp( int argc, char **argv )
{
   CMD_BLOCK cmdBlock;

   //
   // Say Hello
   //
   TTCP_LogMsg( "PCAUSA Test TCP Utility V%s\n", PCATTCP_VERSION );

   //
   // Fetch Command/Options For This Test
   //
   TTCP_SetConfigDefaults( &cmdBlock );

   if( TTCP_ParseCommandLine( &cmdBlock, argc, argv ) != 0 )
   {
      return( 0 );
   }

#if 0
   //
   // Start Winsock 2
   //
   if( WSAStartup( MAKEWORD(0x02,0x00), &g_WsaData ) == SOCKET_ERROR )
   {
      ttcp_print_usage();
      TTCP_ExitTestOnWSAError( NULL, "WSAStartup" );
   }
#endif   

   //
   // Set The Ctrl-C Handler
   //
   //SetConsoleCtrlHandler( CtrlHandler, TRUE );

   if( cmdBlock.m_bOptMultiReceiver )
   {
      TTCP_LogMsg( "  Threading   : Multithreaded\r\n");
   }

   //
   // Dispatch To Specialized Procedues
   //
   if( cmdBlock.m_bTransmit )
   {
      DWORD nResult;

      if( cmdBlock.m_Protocol == IPPROTO_UDP )
      {
         nResult = TTCP_TransmitUDP( &cmdBlock, argv[argc - 1] );
      }
      else
      {
         nResult = TTCP_TransmitTCP( &cmdBlock, argv[argc - 1] );
      }
   }
   else
   {
      DWORD nResult;

#if 0
      if( cmdBlock.m_bOptMultiReceiver )
      {
         if( cmdBlock.m_Protocol == IPPROTO_UDP )
         {
            nResult = TTCP_ReceiveUDP( &cmdBlock );
         }
         else
         {
            HANDLE   RxThreadHandles[ 2 ];   // Thread Handles
            DWORD    tid[2];                 // Thread IDs

            //
            // Run TCP Connection Listener In New Thread
            //
            cmdBlock.m_Protocol = IPPROTO_TCP;
            RxThreadHandles[0] = CreateThread(
                                       NULL,
                                       0,
                                       (LPTHREAD_START_ROUTINE )TTCP_ListenTCP,
                                       &cmdBlock,
                                       0,
                                       &tid[0]
                                       );

            cmdBlock.m_Protocol = IPPROTO_UDP;
            nResult = TTCP_ReceiveUDP( &cmdBlock );
         }
     }
      else
#endif        
      {
         if( cmdBlock.m_Protocol == IPPROTO_UDP )
         {
            nResult = TTCP_ReceiveUDP( &cmdBlock );
         }
         else
         {
            nResult = TTCP_ListenTCP( &cmdBlock );
         }
      }
   }

   //WSACleanup();

   return( 0 );
}

/*
 *			N R E A D
 */
int TTCP_Nread( PTEST_BLOCK pTBlk, int count )
{
   SOCKADDR_IN from;
   int len = sizeof(from);
   register int cnt;

   if( pTBlk->m_Cmd.m_Protocol == IPPROTO_UDP )
   {
		cnt = recvfrom( pTBlk->m_Socket_fd,
               pTBlk->m_pBuf, count, 0,
               (PSOCKADDR )&from,
               (u32_t *)&len
               );

      pTBlk->m_Cmd.m_RemoteAddr.sin_addr.s_addr = from.sin_addr.s_addr;
      pTBlk->m_Cmd.m_RemoteAddr.sin_port = from.sin_port;

		pTBlk->m_numCalls++;
   }
   else
   {
		if( b_flag )
      {
         cnt = TTCP_mread( pTBlk, count );	/* fill buf */
      }
      else
      {
         cnt = recv( pTBlk->m_Socket_fd, pTBlk->m_pBuf, count, 0 );

         if( cnt == SOCKET_ERROR )
         {
            int nError = WSAGetLastError();
            TTCP_LogMsg("%s,err=%d\r\n",__func__, nError);
         }
         pTBlk->m_numCalls++;
      }
	}

	if( pTBlk->m_Cmd.m_bTouchRecvData && cnt > 0 )
   {
		register int c = cnt, sum;
		register char *b = pTBlk->m_pBuf;
        sum = 0;
		while (c--)
			sum += *b++;
	}

	return(cnt);
}

/*
 *			N W R I T E
 */
int TTCP_Nwrite( PTEST_BLOCK pTBlk, int count )
{
   register int cnt;

   //
   // Introduce Write Delay After First Write Call
   //
   if( pTBlk->m_Cmd.m_nOptWriteDelay && pTBlk->m_numCalls > 0 )
   {
      OS_MsDelay( pTBlk->m_Cmd.m_nOptWriteDelay );
   }

   if( pTBlk->m_Cmd.m_Protocol == IPPROTO_UDP )
   {
again:
      cnt = sendto( pTBlk->m_Socket_fd, pTBlk->m_pBuf, count, 0,
               (PSOCKADDR )&pTBlk->m_Cmd.m_RemoteAddr,
               sizeof(pTBlk->m_Cmd.m_RemoteAddr)
               );

      pTBlk->m_numCalls++;

      if( cnt == SOCKET_ERROR && ((WSAGetLastError() == ENOBUFS) || (WSAGetLastError() == ENOMEM)))
      {
         delay(18000);
         l3_errno = 0;
         goto again;
      }
   }
   else
   {
      cnt = send( pTBlk->m_Socket_fd, pTBlk->m_pBuf, count, 0 );
      pTBlk->m_numCalls++;
   }

   if(cnt<=0)
   {
	   TTCP_LogMsg( "fun sendto error cnt:%d ,error=%d\r\n", cnt,WSAGetLastError());
   }

   return(cnt);
}

void delay( int us )
{
	struct timeval tv;

	tv.tv_sec = 0;
	tv.tv_usec = us;
	select( 1, (fd_set *)0, (fd_set *)0, (fd_set *)0, &tv );
}

/*
 *			M R E A D
 *
 * This function performs the function of a read(II) but will
 * call read(II) multiple times in order to get the requested
 * number of characters.  This can be necessary because
 * network connections don't deliver data with the same
 * grouping as it is written with.  Written by Robert S. Miles, BRL.
 */
int TTCP_mread( PTEST_BLOCK pTBlk, unsigned n)
{
   char *bufp = pTBlk->m_pBuf;
   register unsigned	count = 0;
   register int		nread;

   do
   {
		nread = recv( pTBlk->m_Socket_fd, bufp, n-count, 0);

		pTBlk->m_numCalls++;

		if(nread < 0)  {
			//perror("ttcp_mread");
			return(-1);
		}
		if(nread == 0)
			return((int)count);

		count += (unsigned)nread;

		bufp += nread;
	 }
      while(count < n);

	return((int)count);
}

#define END(x)	{while(*x) x++;}

char *outfmt( double b )
{
   static char obuf[50];

   switch (fmt) {
	case 'G':
	    sprintf(obuf, "%.2f GB", b / 1024.0 / 1024.0 / 1024.0);
	    break;
	default:
	case 'K':
	    sprintf(obuf, "%.2f KB", b / 1024.0);
	    break;
	case 'M':
	    sprintf(obuf, "%.2f MB", b / 1024.0 / 1024.0);
	    break;
	case 'g':
	    sprintf(obuf, "%.2f Gbit", b * 8.0 / 1024.0 / 1024.0 / 1024.0);
	    break;
	case 'k':
	    sprintf(obuf, "%.2f Kbit", b * 8.0 / 1024.0);
	    break;
	case 'm':
	    sprintf(obuf, "%.2f Mbit", b * 8.0 / 1024.0 / 1024.0);
	    break;
    }

    return obuf;
}

