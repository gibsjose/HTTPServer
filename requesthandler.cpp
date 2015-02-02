#include "requesthandler.h"

void * requesthandler_run(void * aData_ptr)
{
  //Dereference the void* pointer to be an int*.
  server_info_t * server_info = (server_info_t *) aData_ptr;
  int lSocketFD = server_info->socket;
  std::string docroot = server_info->docroot;

  char line[SOCKET_BUFFER_BYTES];
  rqheader_t r_header;

  // Set a read timeout on the socket.
  struct timeval timeout;
  timeout.tv_sec = SOCKET_READ_TIMEOUT_SEC;
  timeout.tv_usec = 0;

  setsockopt(lSocketFD, SOL_SOCKET, SO_RCVTIMEO, &timeout,sizeof(struct timeval));

  while(1)
  {
    printf("Receiving...\n");
    size_t lRetVal_recv = recv(lSocketFD, line, SOCKET_BUFFER_BYTES, 0);
    if (0 == lRetVal_recv)
    {
      // Client likely closed the connection.
      pthread_exit(NULL);
    }
    else if(-1 == lRetVal_recv)
    {
      //some other error
      fprintf(stderr, "recv(): Error\n");
      perror(strerror(errno));
      pthread_exit(NULL);
    }
    else
    {
      printf("Received %lu bytes.\n", lRetVal_recv);
    }

    printf("Got from client:\n");
    printf("-----------------------------\n");
    printf("%s\n", line);
    printf("-----------------------------\n");

    //Put together a response header
    //Response header example
    // HTTP/1.1 <status_code> <status_label>
    // HTTP/1.1 200 OK
    // Date: Fri, 31 Dec 1999 23:59:59 GMT
    // Content-Type: text/html
    // Content-Length: 1354

    //1. Parse the entire request header
    // Build up a data stucture describing request
    // use parse_request function
    parse_request(&r_header, line);

    std::string resp;
    std::string filepath = docroot + "/" + r_header.path;

    resp.clear();

    //2. If request type is not implemented (!GET) then build 501 response.
    //   Set the file path to appropriate html response page.
    if(strcmp(r_header.type.c_str(), "GET")) {
        std::cout << "Not a GET request... 501" << std::endl;
        resp = build_501(r_header);
    }

    //3. Check if the file exists. If not send 404 response.
    //   Set the file path to appropriate html response page.
    else if(!file_exists(filepath)) {
        std::cout << "File " << filepath << " not found... 404" << std::endl;
        resp = build_404(r_header);
    }

    //4. Check for if-modified-since header in the request. If found and the requested
    //   file has not been modified, send a 304 response.
    //   Set the file path to appropriate html response page. (probably no page for this so filepath null)
    else if(r_header.header_map.find("if-modified-since") != r_header.header_map.end()) {
        if(modified_since(filepath, r_header.header_map["if-modified-since"])) {
            std::cout << "File " << filepath << " modified since " << r_header.header_map["if-modified-since"] << "... 304" << std::endl;
            resp = build_304(r_header);
        }
    }

    //5. Create status 200 reponse.
    //   Set the file path to appropriate html response page (requested page)
    else {
        resp = build_200(r_header, filepath);
    }

    size_t lRetVal_send = send(lSocketFD, resp.c_str(), resp.length(), 0);
    if(-1 == lRetVal_send) {
      fprintf(stderr, "send(): Error\n");
      perror(strerror(errno));
      pthread_exit(NULL);
    } else {
      printf("Sent %lu bytes.\n", lRetVal_send);
    }

    /*
    * In HTTP 1.1 all connections are considered persistent unless declared otherwise.
    * http://en.wikipedia.org/wiki/HTTP_persistent_connection
    * So close the socket if the Connection: close header is sent.
    */
    if(r_header.header_map["Connection"] == "close") {
        close(lSocketFD);
    }

    pthread_exit(0);

    // Clear the buffer for reading from the client socket.
    memset(&line, 0, SOCKET_BUFFER_BYTES);
  }
}

bool file_exists(const std::string &path) {
    int ret = access(path.c_str(), R_OK);
    return (ret != -1);
}

bool modified_since(const std::string &path, const std::string &time_str) {
    struct stat st;
    stat(path.c_str(), &st);

    time_t modified = st.st_mtime;
    struct tm tm;
    time_t check;

    //Convert the string to a time_t
    strptime(time_str.c_str(), "%a, %d, %b %Y %H:%M:%S GMT\r\n", &tm);

    check = mktime(&tm);

    if(difftime(modified, check) > 0) {
        return true;
    } else {
        return false;
    }
}

void parse_request(rqheader_t* rq, char* buffer)
{
    char *line, *line_end = NULL;
    char *token, *tok_end = NULL;

    line = strtok_r(buffer, "\n", &line_end);
    log_info("line: %s\n", line);
    if(line != NULL) {
        token = strtok_r(line, " ", &tok_end);
        rq->type = std::string(token);

        token = strtok_r(NULL, " ", &tok_end);
        rq->path = std::string(token);

        token = strtok_r(NULL, " ", &tok_end);
        rq->version = std::string(token);
    }

    log_info("type: %s\npath: %s\nversion: %s\n", rq->type.c_str(), rq->path.c_str(), rq->version.c_str());

    while(1) {
        line = strtok_r(NULL, "\n", &line_end);
        if(line == NULL) break;

        //Get key
        char * key = strtok_r(line, ": ", &tok_end);
        if(key == NULL) break;

        //Get value
        char * value = strtok_r(NULL, ": ", &tok_end);
        if(value == NULL) break;

        //Insert the key/value pair into the header map
        rq->header_map.insert(header_pair_t(std::string(key), std::string(value)));
    }
}

std::string build_501(rqheader_t rq) {
    std::ostringstream oss;
    char * date;

    date = get_date_header();

    oss << "HTTP/1.1" << " 501" << " Not Implemented\r\n";
    oss << "Date: " << std::string(date);
    oss << "Content-Type: text/html\r\n";
    oss << "\r\n";

    //Read in 501 error file
    char* file_buffer = (char *)malloc(MAX_FILE_SIZE_BYTES);
    int bytes_read = read_file("error_pages/501.html", file_buffer);

    oss << std::string(file_buffer) << "\r\n";

    free(file_buffer);

    std::string response = oss.str();

    std::cout << response;

    return response;
}

std::string build_404(rqheader_t rq) {
    std::ostringstream oss;
    char * date;

    date = get_date_header();

    oss << "HTTP/1.1" << " 404" << " Not Found\r\n";
    oss << "Date: " << std::string(date);
    oss << "Content-Type: text/html\r\n";
    oss << "\r\n";

    //Read in 404 error file
    char* file_buffer = (char *)malloc(MAX_FILE_SIZE_BYTES);
    int bytes_read = read_file("error_pages/404.html", file_buffer);

    oss << std::string(file_buffer) << "\r\n";

    free(file_buffer);

    std::string response = oss.str();

    std::cout << response;

    return response;
}

std::string build_304(rqheader_t rq) {
    std::ostringstream oss;
    char * date;

    date = get_date_header();

    oss << "HTTP/1.1" << " 304" << " Not Modified\r\n";
    oss << "Date: " << std::string(date);
    oss << "\r\n";

    std::string response = oss.str();
    std::cout << response;

    return response;
}

std::string build_200(rqheader_t rq, const std::string &filepath) {
    std::ostringstream oss;
    char * date;
    char *lastModified;
    std::string contentType;

    date = get_date_header();
    lastModified = get_last_modified(filepath.c_str());
    const char *ext = get_filename_ext(filepath.c_str());

    if(!strcmp(ext, "html")) {
        contentType = "text/html";
    }

    else if(!strcmp(ext, "txt")) {
        contentType = "text/plain";
    }

    else if(!strcmp(ext, "jpg")) {
        contentType = "image/jpg";
    }

    else if(!strcmp(ext, "jpeg")) {
        contentType = "image/jpeg";
    }

    else if(!strcmp(ext, "pdf")) {
        contentType = "application/pdf";
    }

    else {
        contentType = "text/plain";
    }

    //Read in file
    char* file_buffer = (char *)malloc(MAX_FILE_SIZE_BYTES);
    int bytes_read = read_file(filepath.c_str(), file_buffer);

    oss << "HTTP/1.1" << " 200" << " OK\r\n";
    oss << "Date: " << std::string(date);
    oss << "Last-Modified: " << std::string(lastModified);
    oss << "Content-Type: " << contentType << "\r\n";
    oss << "Content-Length: " << bytes_read << "\r\n";
    oss << "\r\n";

    std::cout << "Response (No Content): \n-----------------" << std::endl;
    std::cout << oss.str() << std::endl;

    oss << std::string(file_buffer, bytes_read);

    free(file_buffer);

    std::string response = oss.str();

    std::cout << "response.size() = " << response.size() << std::endl;

    return response;
}

//Header format
//Date: Tue, 15 Nov 1994 08:12:31 GMT
//gmtime(), tm struct
char* get_date_header()
{
    char* buffer = (char *)malloc(100 * sizeof(char));
    time_t current_time = time(NULL);
    struct tm * _tm = gmtime(&current_time);
    strftime(buffer, 500, "%a, %d, %b %Y %H:%M:%S GMT\r\n", _tm);
    return buffer;
}

char *get_last_modified(const char *file) {
    char* buffer = (char *)malloc(100 * sizeof(char));
    struct stat st;
    stat(file, &st);
    struct tm * _tm = gmtime(&(st.st_mtime));
    strftime(buffer, 500, "%a, %d, %b %Y %H:%M:%S GMT\r\n", _tm);
    return buffer;
}

//Taken from SO: http://stackoverflow.com/questions/5309471/getting-file-extension-in-c
const char *get_filename_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}

int read_file(std::string filepath, char * lBuffer)
{
    size_t lTotalBytesRead = 0;
    FILE * lFilePtr = fopen(filepath.c_str(), "rb");

    if(lFilePtr == 0) {
        perror(strerror(errno));
        return -1;
    }
    while (!feof(lFilePtr)) {
        int lNumBytesRead = fread(lBuffer, 1, MAX_FILE_SIZE_BYTES, lFilePtr);
        lTotalBytesRead += lNumBytesRead;
        if(lNumBytesRead == 0) {
            if (ferror(lFilePtr)) {
                printf("There was an error reading the file.\n");
                return -1;
            }
            else {
                //done reading the file
                return lTotalBytesRead;
            }
        }
    }

    return lTotalBytesRead;
}
