#include &lt;stdio.h&gt;
#include &lt;string.h&gt;   //strlen
#include &lt;stdlib.h&gt;
#include &lt;errno.h&gt;
#include &lt;unistd.h&gt;   //close
#include &lt;arpa/inet.h&gt;    //close
#include &lt;sys/types.h&gt;
#include &lt;sys/socket.h&gt;
#include &lt;netinet/in.h&gt;
#include &lt;sys/time.h&gt; //FD_SET, FD_ISSET, FD_ZERO macros
 
#define TRUE   1
#define FALSE  0
#define PORT 8888

int main(int argc , char *argv[])
{
    int opt = TRUE;
    int master_socket , addrlen , new_socket , client_socket[30] , max_clients = 30 , activity, i , valread , sd;
	int max_sd;
    struct sockaddr_in address;
     
    char buffer[1025];  //data buffer of 1K
     
    //set of socket descriptors
    fd_set readfds;
     
    //a message
    char *message = &quot;ECHO Daemon v1.0 \r\n&quot;;
 
    //initialise all client_socket[] to 0 so not checked
    for (i = 0; i &lt; max_clients; i++) 
    {
        client_socket[i] = 0;
    }
     
    //create a master socket
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0) 
    {
        perror(&quot;socket failed&quot;);
        exit(EXIT_FAILURE);
    }
 
    //set master socket to allow multiple connections , this is just a good habit, it will work without this
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&amp;opt, sizeof(opt)) &lt; 0 )
    {
        perror(&quot;setsockopt&quot;);
        exit(EXIT_FAILURE);
    }
 
    //type of socket created
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
     
    //bind the socket to localhost port 8888
    if (bind(master_socket, (struct sockaddr *)&amp;address, sizeof(address))&lt;0) 
    {
        perror(&quot;bind failed&quot;);
        exit(EXIT_FAILURE);
    }
	printf(&quot;Listener on port %d \n&quot;, PORT);
	
    //try to specify maximum of 3 pending connections for the master socket
    if (listen(master_socket, 3) &lt; 0)
    {
        perror(&quot;listen&quot;);
        exit(EXIT_FAILURE);
    }
     
    //accept the incoming connection
    addrlen = sizeof(address);
    puts(&quot;Waiting for connections ...&quot;);
    
	while(TRUE) 
    {
        //clear the socket set
        FD_ZERO(&amp;readfds);
 
        //add master socket to set
        FD_SET(master_socket, &amp;readfds);
        max_sd = master_socket;
		
        //add child sockets to set
        for ( i = 0 ; i &lt; max_clients ; i++) 
        {
            //socket descriptor
			sd = client_socket[i];
            
			//if valid socket descriptor then add to read list
			if(sd &gt; 0)
				FD_SET( sd , &amp;readfds);
            
            //highest file descriptor number, need it for the select function
            if(sd &gt; max_sd)
				max_sd = sd;
        }
 
        //wait for an activity on one of the sockets , timeout is NULL , so wait indefinitely
        activity = select( max_sd + 1 , &amp;readfds , NULL , NULL , NULL);
   
        if ((activity &lt; 0) &amp;&amp; (errno!=EINTR)) 
        {
            printf(&quot;select error&quot;);
        }
         
        //If something happened on the master socket , then its an incoming connection
        if (FD_ISSET(master_socket, &amp;readfds)) 
        {
            if ((new_socket = accept(master_socket, (struct sockaddr *)&amp;address, (socklen_t*)&amp;addrlen))&lt;0)
            {
                perror(&quot;accept&quot;);
                exit(EXIT_FAILURE);
            }
         
            //inform user of socket number - used in send and receive commands
            printf(&quot;New connection , socket fd is %d , ip is : %s , port : %d \n&quot; , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
       
            //send new connection greeting message
            if( send(new_socket, message, strlen(message), 0) != strlen(message) ) 
            {
                perror(&quot;send&quot;);
            }
             
            puts(&quot;Welcome message sent successfully&quot;);
             
            //add new socket to array of sockets
            for (i = 0; i &lt; max_clients; i++) 
            {
                //if position is empty
				if( client_socket[i] == 0 )
                {
                    client_socket[i] = new_socket;
                    printf(&quot;Adding to list of sockets as %d\n&quot; , i);
					
					break;
                }
            }
        }
         
        //else its some IO operation on some other socket :)
        for (i = 0; i &lt; max_clients; i++) 
        {
            sd = client_socket[i];
             
            if (FD_ISSET( sd , &amp;readfds)) 
            {
                //Check if it was for closing , and also read the incoming message
                if ((valread = read( sd , buffer, 1024)) == 0)
                {
                    //Somebody disconnected , get his details and print
                    getpeername(sd , (struct sockaddr*)&amp;address , (socklen_t*)&amp;addrlen);
                    printf(&quot;Host disconnected , ip %s , port %d \n&quot; , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
                     
                    //Close the socket and mark as 0 in list for reuse
                    close( sd );
                    client_socket[i] = 0;
                }
                 
                //Echo back the message that came in
                else
                {
                    //set the string terminating NULL byte on the end of the data read
                    buffer[valread] = '&#92;&#48;';
                    send(sd , buffer , strlen(buffer) , 0 );
                }
            }
        }
    }
     
    return 0;
}
