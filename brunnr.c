#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <sqlite3.h>
#include <getopt.h>
#include <unistd.h>

void trim(char *str);
static int callback(void *NotUsed, int argc, char **argv, char **azColName);
static void print_version();
static void print_help();
void setup_serial();
void setup_db();
void check_file();
void write_db(char *sql);
void parse_incoming(char string[]);
void write_file(char string[]);
void read_data();

const char *program_name = "brunnr";
const char *VERSION = "0.0.2";
char *search = ":"; // token which separates source, target, and message from each other
char *write_method = "stdout"; // directly write message to brunnr
char *read_method = "serial";
char *loop = NULL; // how many times to read from serial port; if NULL, loop forever
char *portname = NULL;
char *file;
char *db_file = "arduino_messages.db";
int wait_time = 25;
int fd, n;

// option definitions
static const struct option longopts[] =
{
  { "help", no_argument, NULL, 'h' },
  { "version", no_argument, NULL, 'v' },
  { "port", required_argument, NULL, 'p' },
  { "file", required_argument, NULL, 'f' },
  { "output", required_argument, NULL, 'o' },
  { "number", required_argument, NULL, 'n' },
  { "write", required_argument, NULL, 'w' },
  { "time-delay", required_argument, NULL, 't' },
  { NULL, 0, NULL, 0 }
};

void setup_serial()
{
  fd = open(portname, O_RDWR | O_NOCTTY | O_NDELAY);

  struct termios toptions;

  tcgetattr(fd, &toptions);

  // 9600 seems to be the standard rate. This should eventually be changed
  // to a command line argument as well.
  cfsetispeed(&toptions, B9600);
  cfsetospeed(&toptions, B9600);
  toptions.c_cflag &= ~PARENB;
  toptions.c_cflag &= ~CSTOPB;
  toptions.c_cflag &= ~CSIZE;
  toptions.c_cflag |= CS8;
  toptions.c_iflag |= ICANON;

  tcsetattr(fd, TCSANOW, &toptions);
  usleep(1000*1000);
  tcflush(fd, TCIOFLUSH);
}

void setup_db()
{
  char *sql = "CREATE TABLE IF NOT EXISTS messages(id INTEGER PRIMARY KEY, source TEXT NOT NULL, target TEXT NOT NULL, message TEXT NOT NULL)";
  write_db(sql);
}

void check_file() {
  printf("Hasn't been implemented yet\n");
  exit(1);
}

int main(int argc, char *argv[])
{
  int optc;
  int t = 0,n = 0,lose = 0;
  program_name = argv[0];

  // sets variables based on command line arguments
  // if switch requires a parameter, append a colon to the letter
  while ((optc = getopt_long (argc, argv, "vhp:f:d:n:w:r:t:", longopts, NULL)) != -1)
  {
    switch (optc)
    {
      case 'v':
        print_version();
        exit(0);
        break;
      case 'h':
        print_help();
        exit(0);
        break;
      case 'p':
        portname = optarg;
        break;
      case 'f':
        file = optarg;
        break;
      case 'd':
        db_file = optarg;
        break;
      case 'n':
        loop = optarg;
        break;
      case 'w':
        if (strcmp(optarg, "file") == 0  || strcmp(optarg, "db") == 0  || strcmp(optarg, "stdout") == 0 ) {
          write_method = optarg;
        } else {
          printf("Invalid write method\n");
          exit(1);
        }
        break;
      case 'r':
        if (strcmp(optarg, "file") == 0) {
          read_method = optarg;
        } else if (strcmp(optarg, "db") == 0) {
          read_method = optarg;
        } else if (strcmp(optarg, "serial") == 0 ) {
          read_method = optarg;
        } else {
          printf("Invalid read method\n");
          exit(1);
        }
        break;
      case 't':
        wait_time = atoi(optarg);
        break;
    }
  }
  if (lose || optind < argc)
  {
    // invalid command line argument
    if (optind < argc) 
      fprintf(stderr, ("%s: extra operand: %\n"),
          program_name, argv[optind]);
    fprintf(stderr, "Try 'brunnr --help' for more information.\n");
    exit(1);
  }
  if (strcmp(read_method, "file") == 0 || strcmp(write_method, "file") == 0 ) {
    check_file();
  }
  if (strcmp(read_method, "db")  == 0 || strcmp(write_method, "db") == 0 ) {
    setup_db();
  }
  if (strcmp(read_method, "serial") == 0 ) {
    if (portname != NULL) {
      setup_serial();
    } else {
      printf("Portname not specified\n");
      exit(1);
   }
  }
 if (loop == NULL) {
    while (1<2) {
      read_data();
    }
  } else {
    for (int i=0; i < atoi(loop); i++) {
      read_data();
    }
  }
  close(fd);
}

void read_data() {
  char buf[255];
  if (strcmp(read_method, "serial") == 0) {
    usleep(wait_time*100);
    n = read(fd, buf, 255);
    buf[n] = 0;
    trim(buf); // otherwise stdout output skips lines because of hidden newlines
  } else if (strcmp(read_method, "file") == 0) {
    printf("Hasn't been implemented yet\n");
    exit(1);
  } else if (strcmp(read_method, "db") == 0) {
    printf("Hasn't been implemented yet\n");
    exit(1);
  }
  if(strlen(buf) > 0) {
    if (strcmp(write_method, "file") == 0) {
      write_file(buf);
    } else if (strcmp(write_method, "db") == 0) {
      parse_incoming(buf);
    } else {
      printf("%s\n", buf);
    }
  }
}

// parse_incoming() separates an incoming string into source, target, and message
// and creates a SQL statement to send to write_db()
void parse_incoming(char string[]) 
{
  char sql[1024];
  
  char *source = strtok(string, search);
  char *target = strtok(NULL, search);
  char *message = strtok(NULL, search);

  snprintf(sql, sizeof(sql), "INSERT INTO messages(id, source, target, message) values(NULL, '%s', '%s', '%s')", source, target, message);
  printf(sql);
  write_db(sql);
}

void write_file(char string[]) {
  printf("Not implemented yet");
}

static int callback(void *NotUsed, int argc, char **argv, char **azColName) 
{
  int i;
  for(i=0;i<argc;i++) {
    printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  }
  printf("\n");
  return 0;
}

// write_db() is used to execute SQL statements on the specified database
// It can execute any valid SQL.
void write_db(char *sql)
{
  sqlite3 *db;
  char *zErrMsg = 0;
  int rc = 0;

  if (db_file == NULL) {
    printf("Database filename hasn't been specified. . .\n");
    exit(1);
  }
  rc = sqlite3_open(db_file, &db);

  if ( rc ) {
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
  }

  rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }

  sqlite3_close(db);
}

// trim() removes all newlines/carriage returns from a string
void trim(char *str) 
{
  char *src, *dst;
  for (src = dst = str; *src != '\0'; src++) {
    *dst = *src;
    if (*dst != '\n') dst++;
  }
  *dst = '\0';
}

static void print_help()
{
  puts ("");
  fprintf(stdout, "Usage; brunnr [OPTION] \n");
  fprintf(stdout, "Interact with arduinos through the serial port.\n");
  puts ("");
  fprintf(stdout, "-f, --file          specify which database file to use\n");
  fprintf(stdout, "-h, --help          print this help message\n");
  fprintf(stdout, "-n, --number        specify number of times to run\n");
  fprintf(stdout, "-o, --output        specify which output to use\n");
  fprintf(stdout, "-p, --port          specify which port to use\n");
  fprintf(stdout, "-t, --time-delay    specify wait time between reads (default: 25, +1 per char)\n");
  fprintf(stdout, "-v, --version       print the current version number\n");
  fprintf(stdout, "-w, --write         manually write message to database\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "Report bugs to https://github.com/projectdelphai/brunnr\n");
  puts ("");
}

static void print_version()
{
  printf("brunnr %s", VERSION);
  puts("");
}
