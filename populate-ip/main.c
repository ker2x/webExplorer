#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <curl/curl.h>
#include "bson.h"
#include "mongo.h"


struct MemoryStruct {
	char *memory;
	size_t size;
};


static size_t
WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
	size_t realsize = size * nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct *)data;
 
	mem->memory = realloc(mem->memory, mem->size + realsize + 1);
	if (mem->memory == NULL) 
	{
		/* out of memory! */ 
		printf("not enough memory (realloc returned NULL)\n");
		exit(EXIT_FAILURE);
	}
 
	memcpy(&(mem->memory[mem->size]), ptr, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;
 
  return realsize;
}


int main() {
    mongo_connection conn[1]; /* ptr */
    mongo_connection_options opts[1];
    mongo_conn_return status;
	bson bs;
	bson_buffer buf;

	CURL *curl;
	CURLcode res;


	char ip[16];
	char httpAddress[24];
	int a,b,c,d = 0;
	
	srand ( (unsigned)time ( NULL ) );


    strcpy( opts->host , "127.0.0.1" );
    opts->port = 27017;


    /* CODE WILL GO HERE */ 
	while(1)
	{
		status = mongo_connect( conn, opts );

		a = 1 + rand() % 222; // we can directly skip 224.0.0.0 and above
		b = rand() % 255;
		c = rand() % 255;
		d = rand() % 255;

		if(a == 10 || a == 127) { continue; }	//Skip 10.0.0.0/8 and 127.0.0.0/8
		if(a > 223) 			{ continue; }	//Skip 224.0.0.0/4 and 240.0.0.0/4
		if(a == 169 && b == 254) 			{ continue; }	//Skip 169.254.0.0/16
		if(a == 172 && b > 15 && b < 32) 	{ continue; } 	//Skip 172.16.0.0/12
		if(a == 192 && b == 168) 			{ continue; }	//Skip 192.168.0.0/16
		if(a == 198 && b > 17 && b < 20) 	{ continue; } 	//Skip 198.18.0.0/15
		if(a == 192 && b == 0 && c == 0)	{ continue; }	//Skip 192.0.0.0/24
		if(a == 192 && b == 0 && c == 2) 	{ continue; }	//Skip 192.0.2.0/24
		if(a == 192 && b == 88 && c == 99)	{ continue; }	//Skip 192.88.99.0/24
		if(a == 198 && b == 51 && c == 100)	{ continue; }	//Skip 198.51.100.0/24
		if(a == 203 && b == 0 && c == 113)	{ continue; }	//Skip 203.0.113.0/24

		//printf("%d.%d.%d.%d\n", a, b, c, d);
		snprintf(ip, sizeof(ip) * sizeof(char), "%d.%d.%d.%d", a, b, c, d);
		snprintf(httpAddress, sizeof(httpAddress) * sizeof(char), "http://%d.%d.%d.%d/", a, b, c, d);
		//printf("%s\n", ip);

		//CURL STUFF
		struct MemoryStruct chunk;
		chunk.memory = malloc(1);  /* will be grown as needed by the realloc above */ 
		chunk.size = 0;    /* no data at this point */ 

		curl_global_init(CURL_GLOBAL_ALL);
		curl = curl_easy_init();
		curl_easy_setopt(curl, CURLOPT_URL, ip);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
		//curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);

		res = curl_easy_perform(curl);
		if(res == CURLE_OK)
		{
			printf("valid %s\n %s\n", ip, chunk.memory);
			bson_buffer_init( &buf);
			bson_append_new_oid( &buf, "_id" );
			bson_append_string( &buf, "ip", ip );
			bson_append_bool( &buf, "valid", 1);
			bson_append_string( &buf, "content", chunk.memory );
			bson_from_buffer( &bs, &buf );
			mongo_insert(conn, "web.explorer",&bs);
			bson_destroy(&bs);
		} else {
			printf("error : %s code : %d \n",ip,res);
			bson_buffer_init( &buf);
			bson_append_new_oid( &buf, "_id" );
			bson_append_string( &buf, "ip", ip );
			bson_append_bool( &buf, "valid", 0);
			bson_append_int(&buf, "errorcode", res);
			//bson_append_string( &buf, "content", chunk.memory );
			bson_from_buffer( &bs, &buf );
			mongo_insert(conn, "web.explorer",&bs);
			bson_destroy(&bs);
		}

		//Clean CURL stuff
		if(chunk.memory) { free(chunk.memory); }
		curl_global_cleanup();

		mongo_destroy( conn );

	}

    printf( "\nconnection closed\n" );

    return 0;
}

