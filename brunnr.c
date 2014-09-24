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
void write_db(char *sql);
void parse_serial(char string[]);
void read_serial();

const char *program_name = "brunnr";
const char *VERSION = "0.0.2";
char *portname = NULL;
char *output = "stdout";
char *db_file = NULL;
char *search = ":"; // token which separates source, target, and message from each other
char *loop = NULL; // how many times to read from serial port; if NULL, loop forever
char *manual_message; // directly write message to brunnr
int manual_write = 1;
int fd, n;

// option definitions
static const struct option longopts[] =
{
  { "help", no_argument, NULL, 'h' },
  { "version", no_argument, NULL, 'v' },
  { "port", required_argument, NULL, 'p' },
  { "file", required_argument, NULL, 'f' },
  { "output", required_argument, NULL, 'o' },
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

int main(int argc, char *argv[])
{
  int optc;
  int t = 0,n = 0,lose = 0;
  program_name = argv[0];

  // sets variables based on command line arguments
  // if switch requires a parameter, append a colon to the letter
  while ((optc = getopt_long (argc, argv, "vhp:f:o:w:n:", longopts, NULL)) != -1)
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
        db_file = optarg;
        break;
      case 'o':
        output = "db";
        break;
      case 'w':
        manual_write = 0;
        manual_message = optarg;
        break;
      case 'n':
        loop = optarg;
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
  if (portname != NULL) {
    setup_serial();
    // set up db only if asked for
    if (output == "db") {
      setup_db();
    }
    // determine number of times to loop before ending
    if (loop == NULL) {
      while (1<2) {
        read_serial();
      }
    } else {
      for (int i=0; i < atoi(loop); i++) {
        read_serial();
      }
    }
  } else {
    if (manual_write == 0) {
      setup_db();
      parse_serial(manual_message);
    }
    printf("Portname not specified\n");
  }

  close(fd);
}

void read_serial()
{
  usleep(1000*1000);
  char buf[255];
  n = read(fd, buf, 255);
  buf[n] = 0;
  trim(buf); // otherwise stdout output skips lines because of hidden newlines
  if(strlen(buf) > 0) {
    if (output == "stdout") {
      printf("%s\n", buf);
    } else if (output == "db") {
      parse_serial(buf);
    } else {
      printf("Output mode not supported. . .\n");
    }
  }
}

// parse_serial() separates an incoming string into source, target, and message
// and creates a SQL statement to send to write_db()
void parse_serial(char string[]) 
{
  char sql[1024];
  
  char *source = strtok(string, search);
  char *target = strtok(NULL, search);
  char *message = strtok(NULL, search);

  snprintf(sql, sizeof(sql), "INSERT INTO messages(id, source, target, message) values(NULL, '%s', '%s', '%s')", source, target, message);
  printf(sql);
  write_db(sql);
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
  fprintf(stdout, "-v, --version       print the current version number\n");
  fprintf(stdout, "-w, --write         manually write message across port\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "Report bugs to https://github.com/projectdelphai/brunnr\n");
  puts ("");
}

static void print_version()
{
  printf("brunnr %s", VERSION);
  puts("");
}
