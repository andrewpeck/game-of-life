/*-----------------------------------------------------------------------------
 *  Author: Andrew Peck
 *  Assignment: Game of Life
 *-----------------------------------------------------------------------------*/

//#include <iostream>
#include <ncurses.h>
#include <fstream>
#include <signal.h>
#include <cstdlib>
#include <cstring>

using namespace std;

//Set to something greater than the maximum possible terminal width and height;
//only the true width and height (determined through getmaxyx) is used; 
const int height = 150;
const int width = 150;

void makeRandom (bool array[][width], int h, int w);
void PrintGrid (int w, int h, bool array[][width]);
void ToggleCell (bool grid[][width], int x, int y);
int CountNeighbors (bool grid[][width], int x, int y, int h, int w);
void MarkDeaths (int count, int death_birth[][width], int x, int y);
void menu (int ch, int death_birth[][width], bool grid[][width], int &cur_x,
	   int &cur_y, int h, int w, bool & circular, char *fname);
void RunGame (int death_birth[][width], bool grid[][width], int h, int w);
void CircularCopy (bool grid[][width], int h, int w);
void SaveGame (bool grid[][width], int h, int w, char *fname);
void LoadGame (bool grid[][width], char *fname, int h, int w);
void DisplayHelp (void);
void DeleteGrid (bool grid[][width], int h, int w);
void DrawGlider (bool grid[][width], int cur_x, int cur_y);
void ChangeFile (char *fname);
void LWSS (bool grid[][width], int cur_x, int cur_y);
int
main ()
{
  initscr ();			//Initialize Screen 
  cbreak ();			//Set cbreak, aka non-buffered 
  input noecho ();		//Don't echo input keypad 
  (stdscr, TRUE);		//Gives keyboard magic (e.g. KEY_LEFT) 
  start_color ();		//Color!!!  
  init_pair (2, COLOR_BLACK, COLOR_BLUE);	//Set color scheme 
  init_pair (1, COLOR_BLACK, COLOR_RED);	//Set color scheme 

  bool grid[height][width] = { {0}, {0} };	//INITIALIZE GRID TO BLANK 
  int death_birth[height][width] = { {0}, {0} };	//INITIALIZE GRID TO BLANK 
  bool circular = true;		//Toggles circular mode 
  int h, w;
  getmaxyx (stdscr, h, w);	//Gets the current screen size and stores in h, w;
  int cur_x = w / 2, cur_y = h / 2;	//Put the cursor in center of the screen (since it is annoying otherwise) 
  char fname[20] = "default.sav";
  ifstream initialize;
  initialize.open (".gameoflife");
  if (initialize.fail ())
    {
      LoadGame (grid, "default.sav", h, w);
      ofstream savefile;
      savefile.open (".gameoflife");
      savefile << fname;
      savefile.close ();
    }
  else
    {
      initialize >> fname;
      initialize.close ();
      LoadGame (grid, fname, h, w);
      ofstream savefile;
      savefile.open (".gameoflife");
      savefile << fname;
      savefile.close ();
    }
  do
    {
      timeout (0);
      //Set timeout to 0 (aka blocking input--will wait for input before looping) 
      getmaxyx (stdscr, h, w);	//Determine the current width of the console 
      mvprintw (0, 2, "[[");
      printw (fname);
      printw ("]] ");
      printw
	("[F1 - Help] [P to Play] [SPACE to toggle] [S - Save] [L - Load] [F - Change file Name]");
      //Display menu 
      PrintGrid (w, h, grid);	//Print Grid to screen move (cur_y, cur_x);     
      //Move Cursor 
      wrefresh (stdscr);	//Refresh Screen 
      int ch = wgetch (stdscr);	//Listen for a keypress 
      menu (ch, death_birth, grid, cur_x, cur_y, h, w, circular, fname);
      move (cur_y, cur_x);	//Move Cursor 
      wrefresh (stdscr);	//Refresh Screen 
    }
  while (1);
  endwin ();
}

void
RunGame (int death_birth[][width], bool grid[][width], int h, int w)
{
  for (int i = 1; i < width; i++)
    {
      for (int j = 1; j < height; j++)
	{
	  int count;
	  count = CountNeighbors (grid, i, j, h, w);
	  MarkDeaths (count, death_birth, i, j);
  }} for (int i = 1; i < width; i++)
    {
      for (int j = 1; j < height; j++)
	{
	  if (death_birth[i][j] == -1)
	    grid[i][j] = false;
	  else if (death_birth[i][j] == 1)
	    grid[i][j] = true;
	}
    }
}

void
menu (int ch, int death_birth[][width], bool grid[][width], int &cur_x,
      int &cur_y, int h, int w, bool & circular, char *fname)
{
  switch (ch)
    {
    case '1':
      DrawGlider (grid, cur_x, cur_y);
      break;
    case '2':
      LWSS (grid, cur_x, cur_y);
      break;

    case 'F':
    case 'f':
      ChangeFile (fname);
      break;
    case 'R':
    case 'r':
      makeRandom (grid, h, w);
      break;
    case 'C':
    case 'c':
      circular = !circular;
      break;
    case 10:
      {
	for (int i = 1; i < w; i++)
//Clear Grid 
	  grid[i][h] = false;
	for (int j = 1; j < h; j++)	//Clear Grid 
	  grid[j][w] = false;
	if (circular)
	  CircularCopy (grid, h, w);
	RunGame (death_birth, grid, h, w);
      }
      break;

    case ' ':
      ToggleCell (grid, cur_x, cur_y);
      break;

    case KEY_UP:
      if (cur_y > 1)
	cur_y -= 1;
      else if (circular)
	cur_y = h - 1;
      break;

    case KEY_DOWN:
      if (cur_y < h - 1)
	cur_y += 1;
      else if (circular)
	cur_y = 1;
      break;

    case KEY_LEFT:
      if (cur_x > 1)
	cur_x -= 1;
      else if (circular)
	cur_x = w - 1;
      break;

    case KEY_RIGHT:
      if (cur_x < w - 1)
	cur_x += 1;
      else if (circular)
	cur_x = 1;
      break;

    case 'p':
      do
	{
	  timeout (3);
	  move (cur_y, cur_x);
	  for (int i = 1; i < w; i++)	//Clears the gutters
	    grid[i][h] = false;
	  for (int j = 1; j < h; j++)	//Clears the gutters
	    grid[j][w] = false;
	  if (circular)
	    CircularCopy (grid, h, w);	//If circular, copy around
	  PrintGrid (w, h, grid);
	  move (cur_y, cur_x);

	  wrefresh (stdscr);
	  RunGame (death_birth, grid, h, w);

	  int ch = getch ();	//wgetch (stdscr);
	  if ('p' == ch)
	    return;
	  else
	    menu (ch, death_birth, grid, cur_x, cur_y, h, w, circular, fname);
	}
      while (1);
      break;
    case KEY_F (1):
      DisplayHelp ();
      break;
    case 'S':
    case 's':
      SaveGame (grid, height, width, fname);
      break;
    case 'L':
    case 'l':
      LoadGame (grid, fname, h, w);
/*       cur_x = w / 2;
 *       cur_y = h / 2;
 */
      break;
    case KEY_DC:
      DeleteGrid (grid, h, w);
      break;
    }
}

void
PrintGrid (int w, int h, bool array[][width])
{
  for (int i = 1; i < w; i++)
    {
      for (int j = 1; j < h; j++)
	{
	  if (array[i][j] == false)
	    {
	      attron (COLOR_PAIR (2));
	      mvprintw (j, i, " ");
	      attroff (COLOR_PAIR (2));
	    }
	  else if (array[i][j] == true)
	    {
	      attron (COLOR_PAIR (1));
	      mvprintw (j, i, "@");
	      attroff (COLOR_PAIR (1));
	    }
	}
    }
}

void
ToggleCell (bool grid[][width], int x, int y)
{

  /*  Toggles a cell.. If it is ture, turns it false. 
   *  If false, turns it true..
   */

  grid[x][y] = !grid[x][y];

}

int
CountNeighbors (bool grid[][width], int x, int y, int h, int w)
{

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  CountNeighbors
 *
 *  Description:  Counts the number of living neighbors around a cell in the grid. 
 *  
 *    Arguments:  
 *		  
 *	 Return:  VOID
 * =====================================================================================
 */

  int n = 0;

  for (int i = -1; i <= 1; i++)
    {
      for (int j = -1; j <= 1; j++)
	{
	  if ((i == 0) && (j == 0))
	    n = n;
	  else if ((x + i < 0) || (x + i > w) || (y + j < 0) || (y + j > h))
	    n = n;
	  else if (grid[x + i][y + j])
	    {
	      n++;
	    }
	}
    }
  return n;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  MarkDeaths
 *
 *  Description:  Marks onto a second grid the change in state of a cell; depending on 
 *		  the count of neighbours, the cells will either live, die, 
 *		  or remain the same. 
 *  
 *    Arguments:  grid (the grid the game is stored on); x & y (the x & y
 *		  position of the cell being considered)
 *
 *	 Return:  VOID
 * =====================================================================================
 */
void
MarkDeaths (int count, int death_birth[][width], int x, int y)
{
  if (count < 2)
    death_birth[x][y] = -1;
  else if (count > 3)
    death_birth[x][y] = -1;
  else if (count == 3)
    death_birth[x][y] = 1;
  else
    death_birth[x][y] = 0;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  CircularCopy
 *
 *  Description:  Copies the top row to a buffer on the bottom; the left column to a 
 *		  buffer on the right; the bottom row to a buffer on the top; the right
 *		  column to a buffer on the top. 
 *  
 *    Arguments:  grid (the grid the game is stored on); h (the current height of 
 *		  the grid); w (the current width of the file). 
 *
 *	 Return:  VOID
 * =====================================================================================
 */
void
CircularCopy (bool grid[][width], int h, int w)
{


  for (int i = 1; i < w; i++)
    {
      grid[i][h] = grid[i][1];
      grid[i][0] = grid[i][h - 1];
    }

  for (int j = 1; j < h; j++)
    {
      grid[0][j] = grid[w - 1][j];
      grid[w][j] = grid[1][j];
    }

  grid[h][w] = grid[1][1];
  grid[0][0] = grid[h - 1][w - 1];
  grid[h][0] = grid[1][w - 1];
  grid[0][w] = grid[h - 1][1];
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  SaveGame
 *
 *  Description:  Save the current game into a file. 
 *  
 *    Arguments:  grid (the grid the game is stored on); fname (the name of the
 *		  file being saved to); h (the current height of the grid); 
 *		  w (the current width of the file). 
 *
 *	 Return:  VOID
 * =====================================================================================
 */
void
SaveGame (bool grid[][width], int h, int w, char *fname)
{

  ofstream savefile;
  savefile.open (fname);

  for (int i = 0; i < h; i++)
    {
      for (int j = 0; j < w; j++)
	{
	  if (grid[i][j] == true)
	    savefile << i << " " << j << endl;
	}
    }
  savefile.close ();
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  LoadGame
 *
 *  Description:  Loads a saved game into the grid. 
 *  
 *    Arguments:  grid (the grid the game is stored on); fname (the name of the
 *		  file being loaded); h (the current height of the grid); 
 *		  w (the current width of the file). 
 *
 *	 Return:  VOID
 * =====================================================================================
 */
void
LoadGame (bool grid[][width], char *fname, int h, int w)
{
  ifstream savefile;
  savefile.open (fname);
  if (!savefile.fail ())
    {
      DeleteGrid (grid, h, w);
      int t1, t2;
      while (!savefile.eof ())
	{
	  savefile >> t1;
	  savefile >> t2;
	  grid[t1][t2] = true;
	}
      ofstream savefile;
      savefile.open (".gameoflife");
      savefile << fname;
      savefile.close ();
    }
  else
    {
      clear ();
      move (10, 15);
      printw ("File not found.");
      timeout (-1);
      getch ();
      clear ();
    }
  savefile.close ();
  wrefresh (stdscr);		//Refresh Screen
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  DisplayHelp
 *
 *  Description:  Displays a help menu. 
 *  
 *    Arguments:  VOID
 *
 *	 Return:  VOID
 * =====================================================================================
 */
void
DisplayHelp (void)
{
  erase ();
  nocbreak ();
  timeout (-1);
  mvprintw (1, 0, "   [P] - Play\n");
  mvprintw (2, 0, "   [RET] - Move forward one generation\n ");
  mvprintw (3, 0,
	    "   [SPACE] - Toggle the cell under the cursor from living to dead, or vice versa\n");
  mvprintw (4, 0, "   [C] Circular - Toggles circular mode ON and OFF.");
  mvprintw (5, 0, "   [S] - Saves the current grid to a file.");
  mvprintw (6, 0, "   [L] - Load a saved pattern from file.");
  mvprintw (7, 0, "   [DEL] - Delete the current grid.");
  mvprintw (8, 0, "   [R] - Random; generates a random grid");
  mvprintw (9, 0, "   [1] - Fire a glider");
  mvprintw (10, 0, "   [2] - Fire a Light-weight Space Ship");
  getch ();
  erase ();
  cbreak ();
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  DeleteGrid
 *
 *  Description:  Fires a glider 
 *  
 *    Arguments:  grid (that you are playing on); cur_x, cur_y - the location of cursor
 *
 *	 Return:  VOID
 * =====================================================================================
 */
void
DeleteGrid (bool grid[][width], int w, int h)
{
  for (int i = 1; i < h; i++)
    for (int j = 1; j < w; j++)
      grid[i][j] = 0;
  PrintGrid (h, w, grid);
  wrefresh (stdscr);		//Refresh Screen
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  DrawGlider
 *
 *  Description:  Fires a glider 
 *  
 *    Arguments:  grid (that you are playing on); cur_x, cur_y - the location of cursor
 *
 *	 Return:  VOID
 * =====================================================================================
 */
void
DrawGlider (bool grid[][width], int cur_x, int cur_y)
{
  grid[cur_x][cur_y] = true;
  grid[cur_x + 1][cur_y] = true;
  grid[cur_x + 2][cur_y] = true;
  grid[cur_x + 2][cur_y - 1] = true;
  grid[cur_x + 1][cur_y - 2] = true;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  LWSS
 *
 *  Description:  Fires a Light-weight Space Ship
 *  
 *    Arguments:  grid (that you are playing on); cur_x, cur_y - the location of cursor
 *
 *	 Return:  VOID
 * =====================================================================================
 */
void
LWSS (bool grid[][width], int cur_x, int cur_y)
{
  grid[cur_x][cur_y] = true;
  grid[cur_x][cur_y + 2] = true;
  grid[cur_x + 1][cur_y + 3] = true;
  grid[cur_x + 2][cur_y + 3] = true;
  grid[cur_x + 3][cur_y + 3] = true;
  grid[cur_x + 4][cur_y + 3] = true;
  grid[cur_x + 4][cur_y + 2] = true;
  grid[cur_x + 4][cur_y + 1] = true;
  grid[cur_x + 3][cur_y] = true;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  makeRandom
 *
 *  Description:  makeRandom sets a number (equal to the width / 3) of cells in
 *		  a boolean array to true. The location of these cells is random, and
 *		  determined by the rand() function. 
 *
 *    Arguments:  array (an array), h (the height of the grid), w (the width of the grid).
 *       
 *       Return:  VOID
 * =====================================================================================
 */
void
makeRandom (bool array[][width], int h, int w)
{
  int x, y;
  srand (time (NULL));
  for (int i = 0; i < w * 3; i++)
    {
      x = rand () % (w);
      y = rand () % (h);
      array[x][y] = true;
    }
  PrintGrid (w, h, array);	//Print grid
  wrefresh (stdscr);		//Refresh Screen
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  ChangeFile
 *
 *  Description:  Changes the name of the working file. Passes by reference the
 *		  new name of the cstring. 
 *  
 *    Arguments:  The cstring fname; the current name of the file. 
 *
 *	 Return:  VOID
 * =====================================================================================
 */
void
ChangeFile (char *fname)
{
  erase ();
  echo ();
  nocbreak ();
  timeout (-1);
  mvprintw (9, 15, "Current file name: ");
  printw (fname);
  mvprintw (10, 15, "Change current file name to: ");
  getstr (fname);
  erase ();
  noecho ();
  cbreak ();
  wrefresh (stdscr);
}
