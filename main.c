/* 
 * Tasker will help you managing your IRL tasks. 
 * I wasted time to create this instead of study LMAO. 
*/

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <string.h>

struct task
{
  char name[64];
  double progress;

  unsigned id;
  unsigned long date_added;
};

long int file_size(FILE *file)
{
  fseek(file, 0L, SEEK_END); 
  long int res = ftell(file); 
  fseek(file, 0L, SEEK_SET);

  return res;
}

struct task *tokenize_line(char line[1024])
{
  struct task *task = malloc(sizeof(struct task));
  char *token = strtok(line, ",");
  int n = 0;
  while (token != NULL)
  {
    switch (n)
    {
      case 0:
        strcpy(task->name, token);
        break;
      case 1:
        task->progress = atof(token);
        break;
      case 2:
        task->id = atoi(token);
        break;
      case 3:
        task->date_added = atoi(token);
        break;
      default:
        break;
    }
    token = strtok(NULL, ",");
    n++;
  }
  
  if (n<4)
  {
    free(task);
    return NULL;
  }

  return task;
}

int load_tasks(unsigned uid, struct task **_tasks)
{
  struct task *tasks = malloc(sizeof(struct task *));
  char filename[64];
  sprintf(filename, "U_%u.tasks", uid);
  
  FILE *file = fopen(filename, "r");
  if (file == NULL)
  {
    printf("Error opening file\n");
    return -1;
  }
  char line[1024];
  int r = 0;
  while (fgets(line, sizeof(line), file) != NULL)
  {
    if (r == 0)
    {
      r++;
      continue;
    }

    tasks = realloc(tasks, (r) * sizeof(struct task));


  tasks[r-1] = *tokenize_line(line);
  if (&tasks[r-1] == NULL)
  {
    printf("Error parsing line\n");
    free(tasks);
    fclose(file);
    _tasks = NULL;
    return -1;
  }
  r++;
  }
  fclose(file);  

  if (_tasks != NULL)
    *_tasks = tasks;
  return r-2;
}

struct task *add_task(char name[64], unsigned uid)
{
  struct task *task = malloc(sizeof(struct task));
  
  if (!task)
    return NULL;

  int r = load_tasks(uid, NULL);
  int id = r + 1;

  printf("Task ID: %d\n", id);

  strcpy(task->name, name);
  task->progress = 0.0;
  task->id = id;
  task->date_added = (unsigned long)time(NULL);

  return task;
}

void save_task(unsigned uid, struct task *task)
{
  char filename[64];
  sprintf(filename, "U_%u.tasks", uid);

  FILE *file = fopen(filename, "a+");
  if (file == NULL)
  {
    printf("Error opening file\n");
  }
  else 
  {
    long int res = file_size(file);
    if (res == 0)
    {
      fprintf(file, "Name, Progress, ID, Date Added\n");
    }
  }

  fprintf(file, "%s, %f, %u, %lu\n", task->name, task->progress, task->id, task->date_added);
  fclose(file);
}

void save_task_changes(unsigned uid, struct task *task)
{
  char filename[64];
  sprintf(filename, "U_%u.tasks", uid);
  FILE *file = fopen(filename, "rb");
  if (file == NULL)
  {
    printf("Error opening file\n");
  }

  int s = file_size(file)+1;
  char buffer[s];
  fread(buffer, s, s, file);
  buffer[s-1] = '\0';
  fclose(file);

  printf("BUFFER: %s", buffer);
}


void print_task(struct task *task)
{
  printf("\nTask: %s\n", task->name);
  printf("Progress: %f\n", task->progress);
  printf("ID: %u\n", task->id);
  printf("Date Added: %lu\n", task->date_added);
}

void change_progress(struct task *task, double progress)
{
  task->progress = progress;
}

int main()
{
  int UID = 0;
  char name[64] = "CCNA exam";
  struct task *task = add_task(name, UID);
  save_task(UID, task);
  
  save_task_changes(UID, task);

  struct task *tasks;
  int r = load_tasks(UID, &tasks);
  for (int i = 0; i < r; i++)
  {
    print_task(&tasks[i]);
  }

  return 0;
}
