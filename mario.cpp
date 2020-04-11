#include <cstdlib>
#include <iostream>
#include <vector>
#include <unistd.h>
#include <sys/time.h>
#include <string>
#include <vector>

/*
 * This is a Super Mario Simon game
 * 
 * make sure keyboard caps lock is off;
 * press 'a' to move left;
 * press 'd' to move right;
 * press SPACE to jump;
 * press 'e' to start a new round;
 */

#include <X11/Xlib.h>
#include <X11/Xutil.h>

unsigned long now()
{
	timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000000 + tv.tv_usec;
}

int FPS = 32;

using namespace std;

#include "simon.h"

struct XInfo
{
	Display *display;
	int screen;
	Window window;
	Pixmap pixmap; //for buffering animation
	XWindowAttributes w;
	XEvent event; //save the event here
	GC gc;
};

XInfo xinfo;

int buildColor(int red, int green, int blue)
{
	return (((int)red << 16) + ((int)green << 8) + (int)blue);
}

class Displayable
{
  public:
	virtual void paint(XInfo &xinfo) = 0;

	// constructor
	Displayable(int x, int y) : x(x), y(y) {}

	virtual int getX() { return x; };
	virtual int getY() { return y; };
	virtual void setX(int x) { this->x = x; }
	virtual void setY(int y) { this->y = y; }
	virtual ~Displayable() {}

  private:
	int x;
	int y;
};

class Background : public Displayable
{
  public:
	virtual void paint(XInfo &xinfo)
	{
		XCopyArea(xinfo.display, this->pixmap, xinfo.pixmap, White,
				  0, 0, xinfo.w.width, xinfo.w.height, // region of pixmap to copy
				  0, 0);							   // position to put top left corner of pixmap in window
	}

	void draw(XInfo &xinfo)
	{
		XFillRectangle(xinfo.display, this->pixmap, Blue, 0, 0, xinfo.w.width, getY());
		XFillRectangle(xinfo.display, this->pixmap, Brown, 0, getY(), xinfo.w.width, xinfo.w.height - getY());
		int X = 0;
		while (X <= xinfo.w.width)
		{
			for (int i = 0; i < 9; ++i)
			{
				int size = ground[i].size();
				for (int j = 0; j < size; ++j)
				{
					// choose a color to draw the pixel
					GC *gc = nullptr;
					switch (ground.at(i).at(j))
					{
					case Background::BLACK:
						gc = &Black;
						break;
					case Background::BROWN:
						gc = &Brown;
						break;
					case Background::DARKBROWN:
						gc = &DarkBrown;
						break;
					case Background::WHITE:
						gc = &White;
						break;
					case Background::RED:
						gc = &Red;
						break;
					case Background::DARKRED:
						gc = &DarkRed;
						break;
					case Background::BLUE:
						gc = &Blue;
						break;
					case Background::DARKBLUE:
						gc = &DarkBlue;
						break;
					case Background::GREEN:
						gc = &Green;
						break;
					case Background::DARKGREEN:
						gc = &DarkGreen;
						break;
					case Background::YELLOW:
						gc = &Yellow;
						break;
					default:
						break;
					}
					if (gc != nullptr)
						XFillRectangle(xinfo.display, this->pixmap, *gc, getX() + j * pixwidth + X, getY() + i * pixwidth, pixwidth, pixwidth);
				}
			}
			X += 12 * pixwidth;
		}
	}

	Background(int y, XInfo &xinfo)
		: Displayable(0, y),
		  Black{XCreateGC(xinfo.display, xinfo.window, 0, 0)},
		  Brown{XCreateGC(xinfo.display, xinfo.window, 0, 0)},
		  DarkBrown{XCreateGC(xinfo.display, xinfo.window, 0, 0)},
		  White{XCreateGC(xinfo.display, xinfo.window, 0, 0)},
		  Red{XCreateGC(xinfo.display, xinfo.window, 0, 0)},
		  DarkRed{XCreateGC(xinfo.display, xinfo.window, 0, 0)},
		  Blue{XCreateGC(xinfo.display, xinfo.window, 0, 0)},
		  DarkBlue{XCreateGC(xinfo.display, xinfo.window, 0, 0)},
		  Green{XCreateGC(xinfo.display, xinfo.window, 0, 0)},
		  DarkGreen{XCreateGC(xinfo.display, xinfo.window, 0, 0)},
		  Yellow{XCreateGC(xinfo.display, xinfo.window, 0, 0)},
		  pixmap{XCreatePixmap(xinfo.display, xinfo.window, xinfo.w.width, xinfo.w.height, DefaultDepth(xinfo.display, xinfo.screen))},
		  ground{
			  "bbbbbbbbbbbb",
			  "wwwwwwwwwwww",
			  "gggggggggggg",
			  "GGggGGgggggg",
			  "bbGGbbGGggGG",
			  "NNbbNNbbGGbb",
			  "NNNNNNNNbbNN",
			  "  NN  NNNNNN",
			  "        NN"}
	{
		XSetForeground(xinfo.display, Black, 0);
		XSetForeground(xinfo.display, Brown, buildColor(160, 100, 0));
		XSetForeground(xinfo.display, DarkBrown, buildColor(100, 60, 0));
		XSetForeground(xinfo.display, White, -1);
		XSetForeground(xinfo.display, Red, buildColor(255, 70, 70));
		XSetForeground(xinfo.display, DarkRed, buildColor(185, 0, 70));
		XSetForeground(xinfo.display, Blue, buildColor(0, 200, 250));
		XSetForeground(xinfo.display, DarkBlue, buildColor(0, 165, 235));
		XSetForeground(xinfo.display, Green, buildColor(99, 224, 99));
		XSetForeground(xinfo.display, DarkGreen, buildColor(96, 151, 27));
		XSetForeground(xinfo.display, Yellow, buildColor(252, 239, 28));

		XSetBackground(xinfo.display, Black, 0);
		XSetBackground(xinfo.display, Brown, 0);
		XSetBackground(xinfo.display, DarkBrown, 0);
		XSetBackground(xinfo.display, White, 0);
		XSetBackground(xinfo.display, Red, 0);
		XSetBackground(xinfo.display, DarkRed, 0);
		XSetBackground(xinfo.display, Blue, 0);
		XSetBackground(xinfo.display, DarkBlue, 0);
		XSetBackground(xinfo.display, Green, 0);
		XSetBackground(xinfo.display, DarkGreen, 0);
		XSetBackground(xinfo.display, Yellow, 0);

		draw(xinfo);
	}

  private:
	const int pixwidth = 2;
	GC Black;
	GC Brown;
	GC DarkBrown;
	GC White;
	GC Red;
	GC DarkRed;
	GC Blue;
	GC DarkBlue;
	GC Green;
	GC DarkGreen;
	GC Yellow;
	Pixmap pixmap;

	enum Color
	{
		BLACK = 'b',
		BROWN = 'n',
		DARKBROWN = 'N',
		WHITE = 'w',
		RED = 'r',
		DARKRED = 'R',
		BLUE = 'u',
		DARKBLUE = 'U',
		GREEN = 'g',
		DARKGREEN = 'G',
		YELLOW = 'y',
	};

	vector<string> ground;
};

/*
 * Mario class
 *  image size is 16 * 24 pixels
 */
class Mario : public Displayable
{
  public:
	virtual void paint(XInfo &xinfo)
	{
		paint(xinfo, Mario::NONE);
	}

	virtual void paint(XInfo &xinfo, int state)
	{
		vector<string> *character;

		// choose a character image to draw
		switch (state)
		{
		case Mario::LOSE:
			character = &shocked;
			break;
		case Mario::WIN:
			character = &vSign;
			break;
		default:
			if (isMidAir())
			{
				character = &midAir;
			}
			else if (speedX != 0 && steppingFrame >= 0)
			{
				character = &stepping;
			}
			else
			{
				character = &stationary;
			}
			break;
		}

		for (int i = 0; i < 24; ++i)
		{
			int size = (*character)[i].size();
			for (int j = 0; j < size; ++j)
			{
				// choose a color to draw the pixel
				GC *gc = nullptr;
				switch (character->at(i).at(j))
				{
				case Mario::BLACK:
					gc = &Black;
					break;
				case Mario::BROWN:
					gc = &Brown;
					break;
				case Mario::DARKBROWN:
					gc = &DarkBrown;
					break;
				case Mario::WHITE:
					gc = &White;
					break;
				case Mario::RED:
					gc = &Red;
					break;
				case Mario::DARKRED:
					gc = &DarkRed;
					break;
				case Mario::BLUE:
					gc = &Blue;
					break;
				case Mario::DARKBLUE:
					gc = &DarkBlue;
					break;
				case Mario::PINK:
					gc = &Pink;
					break;
				case Mario::DARKPINK:
					gc = &DarkPink;
					break;
				case Mario::YELLOW:
					gc = &Yellow;
					break;
				default:
					break;
				}
				if (gc != nullptr)
					XFillRectangle(xinfo.display, xinfo.pixmap, *gc, getX() + (j - 8) * facingLeft * pixwidth, getY() + (i - 12) * pixwidth, pixwidth, pixwidth);
			}
		}

		XCopyArea(xinfo.display, xinfo.pixmap, xinfo.window, White,
				  0, 0, xinfo.w.width, xinfo.w.height, // region of pixmap to copy
				  0, 0);							   // position to put top left corner of pixmap in window
	}

	// calculate the the next frame position using speed and gravity;
	void updateFrame()
	{
		if (speedX != 0)
			steppingFrame = (steppingFrame < (FPS / steppingFPS)) ? steppingFrame + 1 : -FPS / steppingFPS;

		setY((ground - getY() <= speedY) ? ground : getY() + speedY); // need to check if dropped onto the ground;
		if (isMidAir())
			speedY += gravity;
		if (!isMidAir() && speedY > 0)
			speedY = 0;

		if (speedX != 0)
		{
			setX(getX() + speedX); // need to check border
		}
	}

	// constructor
	Mario(int x, int y, XInfo &xinfo)
		: Displayable(x, y), facingLeft(1), ground(y),
		  Black{XCreateGC(xinfo.display, xinfo.window, 0, 0)},
		  Brown{XCreateGC(xinfo.display, xinfo.window, 0, 0)},
		  DarkBrown{XCreateGC(xinfo.display, xinfo.window, 0, 0)},
		  White{XCreateGC(xinfo.display, xinfo.window, 0, 0)},
		  Red{XCreateGC(xinfo.display, xinfo.window, 0, 0)},
		  DarkRed{XCreateGC(xinfo.display, xinfo.window, 0, 0)},
		  Blue{XCreateGC(xinfo.display, xinfo.window, 0, 0)},
		  DarkBlue{XCreateGC(xinfo.display, xinfo.window, 0, 0)},
		  Pink{XCreateGC(xinfo.display, xinfo.window, 0, 0)},
		  DarkPink{XCreateGC(xinfo.display, xinfo.window, 0, 0)},
		  Yellow{XCreateGC(xinfo.display, xinfo.window, 0, 0)},
		  stationary{
			  "",
			  "",
			  "",
			  "",
			  "    NNNNN",
			  "   NryrrrNN",
			  "   NwyyRRrrN",
			  "  bbbbbRRrRRN",
			  " bbbbbbbbRRRRN",
			  "    PbPbPbbbpN",
			  "  nnpbpbpPbpnpN",
			  " npppppppbbpnPN",
			  " nPPPPPbppbpPbN",
			  "  bbbbbbbpPPbb",
			  "   bbbbPPPnnb",
			  "     NnnnnRN",
			  "    NuuUUrRRN",
			  "   NwuwwUnnnN",
			  "   Nwuwwnwwwn",
			  "   NuuUUUnwwn",
			  "    NUNUUnwwn",
			  "     bnbnnnn",
			  "    bybynnnb",
			  "    bbbbbbbb"},
		  stepping{
			  "",
			  "",
			  "",
			  "    NNNNN",
			  "   NryrrrNN",
			  "   NwyyRRrrN",
			  "  bbbbbRRrRRN",
			  " bbbbbbbbRRRRN",
			  "    PbPbPbbbpN",
			  "  nnpbpbpPbpnpN",
			  " npppppppbbpnPN",
			  " nPPPPPbppbpPbN",
			  "  bbbbbbbpPPbb",
			  "   bbbbPPPnnb",
			  "     NnnnnRRn",
			  "    NuuUUrRRn",
			  " bbNwuwwUwwwn",
			  "bbybwuwwnnwwnb",
			  "bbnbNuUUUnwwnnb",
			  " bbnbNUUUnnnnnb",
			  " bbnb NNNN bynb",
			  "  bb        bb",
			  "",
			  ""},
		  midAir{
			  "  nn",
			  " nwwn",
			  "nwwwwn",
			  "nwwwNNNNN",
			  " nwNryrrrNN",
			  " NnNwyyRRrrN",
			  "  bbbbbRRrRRN",
			  " bbbbbbbbRRRRN",
			  "    PbPbPbbbpN",
			  "  nnpbpbpPbpnpN",
			  " npppppppbbpnPN",
			  " nPPPPPbppbpPbN",
			  "  bbbbbbbpPPbb",
			  "   bbbbPPPnRnnn",
			  "     NnnnnRnwwn",
			  "    NuuUUrnwwwwn",
			  " bbNwuwwUUnwwwwn",
			  "bbybwuwwUUUnwwn",
			  "bbnbNuUUUUUUnnb",
			  " bbnbNUUUNNNNnb",
			  " bbnb NNNN bynb",
			  "  bb        bb",
			  "",
			  ""},
		  shocked{
			  "      NNNN",
			  "    NNryyrNN",
			  "   NrrwyywrrN",
			  "  NrrRbbbbRrrN",
			  "  NrbbbbbbbbrN",
			  "  NbbbbbbbbbbN",
			  "   NbrbrrbrbN",
			  "  bbrrrrrrrrbb",
			  "   brrPPPPrrb",
			  "  bbrPbPPbPrbb",
			  " npbPPppppPPbpn",
			  " npbbppppppbbpn",
			  " nPbpbNNNNbpbPnn",
			  "  npbbbbbbbbpbwn",
			  " nnnpppRRpppnwwn",
			  "nwwrnnprrpnnrrn",
			  "nwbbNnprrpnNrN",
			  " bbbbwnppnuuN",
			  " bbbbwwnnwwUN",
			  " bbbbNUUUwwbbb",
			  " bbbb NNNNbnnyb",
			  "  bb     bnbbbb",
			  "         bbbbb",
			  "          bb",
		  },
		  vSign{
			  "",
			  "",
			  "",
			  "",
			  "       NNNN",
			  "     NNryyrNN",
			  " b bbrrwyywrrN",
			  "bwbwbrRbbbbRrrN",
			  " bwwbbbbbbbbbrN",
			  "bwbbbPPbPPbPPbN",
			  "bbbwbppbppbppbN",
			  "bwbwbbppppppbbpN",
			  " bbbRpbPPPPbpnPn",
			  " NrrRPbbbbbbPnn",
			  "  NrrRpbbbbpRrrN",
			  "  NRrrrrRRrrrNNN",
			  "   bbruuuuuRRwwN",
			  "  bbbbwuuuwwuwwN",
			  "  bbbbwuuuwwUNN",
			  "  bbbbNUUUuUbbb",
			  "  bbbb NNNNbnnyb",
			  "  bbbb    bnbbbb",
			  "   bb     bbbbb",
			  "           bb"}
	{
		XSetForeground(xinfo.display, Black, 0);
		XSetForeground(xinfo.display, Brown, buildColor(140, 95, 0));
		XSetForeground(xinfo.display, DarkBrown, buildColor(100, 0, 0));
		XSetForeground(xinfo.display, White, -1);
		XSetForeground(xinfo.display, Red, buildColor(255, 70, 70));
		XSetForeground(xinfo.display, DarkRed, buildColor(185, 0, 70));
		XSetForeground(xinfo.display, Blue, buildColor(0, 200, 250));
		XSetForeground(xinfo.display, DarkBlue, buildColor(0, 165, 235));
		XSetForeground(xinfo.display, Pink, buildColor(252, 213, 210));
		XSetForeground(xinfo.display, DarkPink, buildColor(252, 160, 177));
		XSetForeground(xinfo.display, Yellow, buildColor(252, 239, 28));

		XSetBackground(xinfo.display, Black, WhitePixel(xinfo.display, xinfo.screen));
		XSetBackground(xinfo.display, Brown, WhitePixel(xinfo.display, xinfo.screen));
		XSetBackground(xinfo.display, DarkBrown, WhitePixel(xinfo.display, xinfo.screen));
		XSetBackground(xinfo.display, White, WhitePixel(xinfo.display, xinfo.screen));
		XSetBackground(xinfo.display, Red, 0);
		XSetBackground(xinfo.display, DarkRed, 0);
		XSetBackground(xinfo.display, Blue, 0);
		XSetBackground(xinfo.display, DarkBlue, 0);
		XSetBackground(xinfo.display, Pink, 0);
		XSetBackground(xinfo.display, DarkPink, 0);
		XSetBackground(xinfo.display, Yellow, 0);
	}
	~Mario() {}

	bool isMidAir()
	{
		return getY() < ground;
	}

	void jump(bool forced = false)
	{
		if (!isMidAir() || forced)
		{
			speedY = -jumpInitialSpeed;
		}
	}

	void runLeft()
	{
		if (speedX > 0)
		{
			speedX = 0;
		}
		else
		{
			speedX = -horizontalSpeed;
			facingLeft = 1;
		}
	}

	void runRight()
	{
		if (speedX < 0)
		{
			speedX = 0;
		}
		else
		{
			speedX = horizontalSpeed;
			facingLeft = -1;
		}
	}

	void runLeftStop()
	{
		if (speedX < 0)
		{
			speedX = 0;
		}
		else if (speedX == 0)
		{
			speedX = horizontalSpeed;
			facingLeft = -1;
		}
	}

	void runRightStop()
	{
		if (speedX > 0)
		{
			speedX = 0;
		}
		else if (speedX == 0)
		{
			speedX = -horizontalSpeed;
			facingLeft = 1;
		}
	}

	int getTopY()
	{
		return getY() - 10 * pixwidth;
	}

	int getTopX()
	{
		return getX() - 3 * facingLeft * pixwidth;
	}

	int getSpeedY()
	{
		return speedY;
	}

	enum State
	{
		WIN,
		LOSE,
		START,
		NONE
	};

  private:
	const int pixwidth = 2;
	int facingLeft;
	GC Black;
	GC Brown;
	GC DarkBrown;
	GC White;
	GC Red;
	GC DarkRed;
	GC Blue;
	GC DarkBlue;
	GC Pink;
	GC DarkPink;
	GC Yellow;
	int steppingFrame = 0;
	const int steppingFPS = 12; // # of frames to show stepping/stationary img when moving
	int speedX = 0;
	const int horizontalSpeed = 200 / FPS;
	int speedY = 0;
	const int jumpInitialSpeed = 850 / FPS;
	const int gravity = 100 / FPS;
	const int ground; // y position where object will stop falling

	enum Color
	{
		BLACK = 'b',
		BROWN = 'n',
		DARKBROWN = 'N',
		WHITE = 'w',
		RED = 'r',
		DARKRED = 'R',
		BLUE = 'u',
		DARKBLUE = 'U',
		PINK = 'p',
		DARKPINK = 'P',
		YELLOW = 'y',
	};

	vector<string> stationary;
	vector<string> stepping;
	vector<string> midAir;
	vector<string> shocked;
	vector<string> vSign;
};

/*
 * MysteryBrick class
 *  acts like a button in simon game
 */
class MysteryBrick : public Displayable
{
  public:
	virtual void paint(XInfo &xinfo)
	{
		if (isMidAir() && hasCoin)
		{
			// draw a coin bumped out of the brick
			XFillRectangle(xinfo.display, xinfo.pixmap, Black, getX() - 6 * pixwidth, coinY - 4 * pixwidth, 12 * pixwidth, 8 * pixwidth);
			XFillRectangle(xinfo.display, xinfo.pixmap, Black, getX() - 5 * pixwidth, coinY - 6 * pixwidth, 10 * pixwidth, 12 * pixwidth);
			XFillRectangle(xinfo.display, xinfo.pixmap, Black, getX() - 4 * pixwidth, coinY - 7 * pixwidth, 8 * pixwidth, 14 * pixwidth);
			XFillRectangle(xinfo.display, xinfo.pixmap, Black, getX() - 2 * pixwidth, coinY - 8 * pixwidth, 4 * pixwidth, 16 * pixwidth);

			XFillRectangle(xinfo.display, xinfo.pixmap, White, getX() - 5 * pixwidth, coinY - 4 * pixwidth, 1 * pixwidth, 8 * pixwidth);
			XFillRectangle(xinfo.display, xinfo.pixmap, White, getX() - 4 * pixwidth, coinY - 6 * pixwidth, 2 * pixwidth, 2 * pixwidth);
			XFillRectangle(xinfo.display, xinfo.pixmap, White, getX() - 2 * pixwidth, coinY - 7 * pixwidth, 4 * pixwidth, 2 * pixwidth);

			XFillRectangle(xinfo.display, xinfo.pixmap, Brown, getX() + 4 * pixwidth, coinY - 4 * pixwidth, 1 * pixwidth, 8 * pixwidth);
			XFillRectangle(xinfo.display, xinfo.pixmap, Brown, getX() - 2 * pixwidth, coinY + 6 * pixwidth, 4 * pixwidth, 1 * pixwidth);
			XFillRectangle(xinfo.display, xinfo.pixmap, Brown, getX() - 2 * pixwidth, coinY - 6 * pixwidth, 6 * pixwidth, 12 * pixwidth);

			XFillRectangle(xinfo.display, xinfo.pixmap, Yellow, getX() - 4 * pixwidth, coinY - 4 * pixwidth, 6 * pixwidth, 10 * pixwidth);
			XFillRectangle(xinfo.display, xinfo.pixmap, Yellow, getX() - 4 * pixwidth, coinY - 4 * pixwidth, 8 * pixwidth, 8 * pixwidth);
			XFillRectangle(xinfo.display, xinfo.pixmap, Yellow, getX() - 3 * pixwidth, coinY - 5 * pixwidth, 6 * pixwidth, 10 * pixwidth);
			XFillRectangle(xinfo.display, xinfo.pixmap, Yellow, getX() - 2 * pixwidth, coinY - 6 * pixwidth, 5 * pixwidth, 2 * pixwidth);

			XFillRectangle(xinfo.display, xinfo.pixmap, LightYellow, getX() - 3 * pixwidth, coinY - 4 * pixwidth, 6 * pixwidth, 8 * pixwidth);
			XFillRectangle(xinfo.display, xinfo.pixmap, LightYellow, getX() - 3 * pixwidth, coinY - 4 * pixwidth, 5 * pixwidth, 9 * pixwidth);
			XFillRectangle(xinfo.display, xinfo.pixmap, LightYellow, getX() - 2 * pixwidth, coinY - 5 * pixwidth, 4 * pixwidth, 10 * pixwidth);

			XFillRectangle(xinfo.display, xinfo.pixmap, White, getX() - 2 * pixwidth, coinY - 4 * pixwidth, 1 * pixwidth, 8 * pixwidth);
			XFillRectangle(xinfo.display, xinfo.pixmap, White, getX() - 1 * pixwidth, coinY - 5 * pixwidth, 2 * pixwidth, 1 * pixwidth);
			XFillRectangle(xinfo.display, xinfo.pixmap, Yellow, getX() - 0 * pixwidth, coinY - 4 * pixwidth, 1 * pixwidth, 6 * pixwidth);
			XFillRectangle(xinfo.display, xinfo.pixmap, Yellow, getX() - 1 * pixwidth, coinY + 2 * pixwidth, 2 * pixwidth, 2 * pixwidth);
			XFillRectangle(xinfo.display, xinfo.pixmap, Black, getX() + 1 * pixwidth, coinY - 4 * pixwidth, 1 * pixwidth, 8 * pixwidth);
			XFillRectangle(xinfo.display, xinfo.pixmap, Black, getX() - 1 * pixwidth, coinY + 4 * pixwidth, 2 * pixwidth, 1 * pixwidth);
		}

		//draw the brick itself
		XFillRectangle(xinfo.display, xinfo.pixmap, Black, getX() + (-8 - 2) * pixwidth, getY() + (-6 - 2) * pixwidth, 20 * pixwidth, 16 * pixwidth);
		XFillRectangle(xinfo.display, xinfo.pixmap, Black, getX() + (-7 - 2) * pixwidth, getY() + (-7 - 2) * pixwidth, 18 * pixwidth, 18 * pixwidth);
		XFillRectangle(xinfo.display, xinfo.pixmap, Black, getX() + (-6 - 2) * pixwidth, getY() + (-8 - 2) * pixwidth, 16 * pixwidth, 20 * pixwidth);

		XFillRectangle(xinfo.display, xinfo.pixmap, DarkBrown, getX() + (-7 - 2) * pixwidth, getY() + (-6 - 2) * pixwidth, 18 * pixwidth, 16 * pixwidth);
		XFillRectangle(xinfo.display, xinfo.pixmap, DarkBrown, getX() + (-6 - 2) * pixwidth, getY() + (-7 - 2) * pixwidth, 16 * pixwidth, 18 * pixwidth);

		XFillRectangle(xinfo.display, xinfo.pixmap, Brown, getX() + (-7 - 2) * pixwidth, getY() + (-6 - 2) * pixwidth, 17 * pixwidth, 15 * pixwidth);
		XFillRectangle(xinfo.display, xinfo.pixmap, Brown, getX() + (-6 - 2) * pixwidth, getY() + (-7 - 2) * pixwidth, 15 * pixwidth, 17 * pixwidth);

		XFillRectangle(xinfo.display, xinfo.pixmap, White, getX() + (-7 - 2) * pixwidth, getY() + (-6 - 2) * pixwidth, 4 * pixwidth, 5 * pixwidth);
		XFillRectangle(xinfo.display, xinfo.pixmap, White, getX() + (-6 - 2) * pixwidth, getY() + (-7 - 2) * pixwidth, 5 * pixwidth, 4 * pixwidth);
		XFillRectangle(xinfo.display, xinfo.pixmap, White, getX() + (-7 - 2) * pixwidth, getY() + (0 - 2) * pixwidth, 1 * pixwidth, 1 * pixwidth);

		XFillRectangle(xinfo.display, xinfo.pixmap, Yellow, getX() + (-6 - 2) * pixwidth, getY() + (-5 - 2) * pixwidth, 15 * pixwidth, 13 * pixwidth);
		XFillRectangle(xinfo.display, xinfo.pixmap, Yellow, getX() + (-5 - 2) * pixwidth, getY() + (-6 - 2) * pixwidth, 13 * pixwidth, 15 * pixwidth);

		XFillRectangle(xinfo.display, xinfo.pixmap, Black, getX() + (-2 - 2) * pixwidth, getY() + (-1 - 2) * pixwidth, 1 * pixwidth, 5 * pixwidth);
		XFillRectangle(xinfo.display, xinfo.pixmap, Black, getX() + (4 - 2) * pixwidth, getY() + (-1 - 2) * pixwidth, 1 * pixwidth, 5 * pixwidth);
	}

	// constructor
	MysteryBrick(int x, int y, XInfo &xinfo, int i)
		: Displayable(x, y), ground(y), coinY(y), index(i),
		  Black{XCreateGC(xinfo.display, xinfo.window, 0, 0)},
		  Brown{XCreateGC(xinfo.display, xinfo.window, 0, 0)},
		  DarkBrown{XCreateGC(xinfo.display, xinfo.window, 0, 0)},
		  White{XCreateGC(xinfo.display, xinfo.window, 0, 0)},
		  Yellow{XCreateGC(xinfo.display, xinfo.window, 0, 0)},
		  LightYellow{XCreateGC(xinfo.display, xinfo.window, 0, 0)}
	{
		XSetForeground(xinfo.display, Black, 0);
		XSetForeground(xinfo.display, Brown, buildColor(160, 100, 0));
		XSetForeground(xinfo.display, DarkBrown, buildColor(100, 60, 0));
		XSetForeground(xinfo.display, White, -1);
		XSetForeground(xinfo.display, Yellow, buildColor(230, 170, 80));
		XSetForeground(xinfo.display, LightYellow, buildColor(252, 233, 100));

		XSetBackground(xinfo.display, Black, WhitePixel(xinfo.display, xinfo.screen));
		XSetBackground(xinfo.display, Brown, WhitePixel(xinfo.display, xinfo.screen));
		XSetBackground(xinfo.display, DarkBrown, WhitePixel(xinfo.display, xinfo.screen));
		XSetBackground(xinfo.display, White, WhitePixel(xinfo.display, xinfo.screen));
		XSetBackground(xinfo.display, Yellow, 0);
		XSetBackground(xinfo.display, LightYellow, 0);
	}
	~MysteryBrick() {}

	bool isMidAir()
	{
		return getY() < ground;
	}

	void jump(bool hasCoin = false)
	{
		if (!isMidAir())
		{
			coinY = getY();
			speedY = -jumpInitialSpeed;
			this->hasCoin = hasCoin;
		}
	}

	bool bumpedBy(Mario &mario, Simon &simon, bool hasCoin = false)
	{
		if (mario.getSpeedY() < 0 && !isMidAir() && mario.getTopX() + 5 * pixwidth > getX() - 10 * pixwidth && mario.getTopX() - 5 * pixwidth < getX() + 9 * pixwidth && mario.getTopY() < getBottom())
		{
			jump();

			this->hasCoin = hasCoin;
			if (hasCoin)
			{
				this->hasCoin = simon.verifyButton(index);
			}
		}
		return this->hasCoin;
	}

	int getBottom()
	{
		return getY() + 9 * pixwidth;
	}

	void updateFrame()
	{
		setY((ground - getY() <= speedY) ? ground : getY() + speedY); // need to check if dropped onto the ground;
		if (isMidAir())
			speedY += gravity;
		if (!isMidAir() && speedY > 0)
			speedY = 0;
		if (speedY < 0)
		{
			coinY += 5 * speedY;
		}
	}

  private:
	int pixwidth = 2;
	GC Black;
	GC Brown;
	GC DarkBrown;
	GC White;
	GC Yellow;
	GC LightYellow;
	int speedY = 0;
	int coinY;
	const int jumpInitialSpeed = 200 / FPS;
	int gravity = 32 / FPS;
	const int ground;
	const int index;
	bool hasCoin;

	vector<string> stationary;
	vector<string> stepping;
	vector<string> midAir;
};

// Initialize X and create a window
void initX(XInfo &xinfo)
{
	XSizeHints hints;
	hints.x = 10;
	hints.y = 10;
	hints.width = 800;
	hints.height = 400;
	hints.min_width = hints.width;
	hints.min_height = hints.height;
	hints.max_width = hints.width;
	hints.max_height = hints.height;
	hints.flags = PPosition | PSize;

	xinfo.display = XOpenDisplay("");
	// Find out some things about the display you're using.
	xinfo.screen = DefaultScreen(xinfo.display);
	long background = WhitePixel(xinfo.display, xinfo.screen);
	long foreground = BlackPixel(xinfo.display, xinfo.screen);
	xinfo.window = XCreateSimpleWindow(
		xinfo.display,					  // display where window appears
		DefaultRootWindow(xinfo.display), // window's parent in window tree
		hints.x, hints.y,				  // upper left corner location
		hints.width, hints.height,		  // size of the window
		2,								  // width of window's border
		foreground,						  // window border colour
		background);					  // window background colour

	// extra window properties like a window title
	XSetStandardProperties(
		xinfo.display,			 // display containing the window
		xinfo.window,			 // window whose properties are set
		"Simon electronic game", // window's title
		"Simon",				 // icon's title
		None,					 // pixmap for the icon
		None, None,				 // applications command line args
		&hints);				 // size hints for the window

	XSelectInput(xinfo.display, xinfo.window,
				 KeyPressMask | KeyReleaseMask | StructureNotifyMask); // select events
	XMapRaised(xinfo.display, xinfo.window);						   // Put window on screen
	XFlush(xinfo.display);

	XGetWindowAttributes(xinfo.display, xinfo.window, &xinfo.w);

	// DOUBLE BUFFER
	// create bimap (pximap) to us a other buffer
	int depth = DefaultDepth(xinfo.display, xinfo.screen);
	xinfo.pixmap = XCreatePixmap(xinfo.display, xinfo.window, xinfo.w.width, xinfo.w.height, depth);

	// create gc for drawing
	xinfo.gc = XCreateGC(xinfo.display, xinfo.window, 0, 0);

	//load a larger font
	XFontStruct *font;
	font = XLoadQueryFont(xinfo.display, "12x24");
	XSetFont(xinfo.display, xinfo.gc, font->fid);

	XSetBackground(xinfo.display, xinfo.gc, WhitePixel(xinfo.display, xinfo.screen));
	XSetForeground(xinfo.display, xinfo.gc, WhitePixel(xinfo.display, xinfo.screen));

	// give server time to setup
	sleep(1);
}

void waitforE(Background &background, Mario &mario, MysteryBrick **bricks, Simon &simon)
{
	int n = simon.getNumButtons();

	// time of last window paint
	unsigned long lastRepaint = 0;

	// event loop
	while (true)
	{
		if (XPending(xinfo.display) > 0)
		{
			XNextEvent(xinfo.display, &xinfo.event);
			if (xinfo.event.type == KeyPress)
			{
				KeySym key;
				char text[10];
				int i = XLookupString((XKeyEvent *)&xinfo.event, text, 10, &key, 0);
				if (i == 1)
				{
					if (text[0] == 'q')
					{
						XFreePixmap(xinfo.display, xinfo.pixmap);
						for (int i = 0; i < n; ++i)
						{
							delete bricks[i];
						}
						XCloseDisplay(xinfo.display);
						exit(0);
					}
					else if (text[0] == ' ')
					{
						mario.jump();
					}
					else if (text[0] == 'a')
					{
						mario.runLeft();
					}
					else if (text[0] == 'd')
					{
						mario.runRight();
					}
					else if (text[0] == 'e')
					{
						break;
					}
				}
			}
			else if (xinfo.event.type == KeyRelease)
			{
				KeySym key;
				char text[10];
				int i = XLookupString((XKeyEvent *)&xinfo.event, text, 10, &key, 0);
				if (i == 1)
				{
					if (text[0] == 'a')
					{
						mario.runLeftStop();
					}
					else if (text[0] == 'd')
					{
						mario.runRightStop();
					}
				}
			}
			else if (xinfo.event.type == ConfigureNotify)
			{
			}
		}

		unsigned long end = now();

		if (end - lastRepaint > 1000000 / FPS)
		{
			// draw displayables
			background.paint(xinfo);
			for (int i = 0; i < n; ++i)
			{
				bricks[i]->paint(xinfo);
				bricks[i]->bumpedBy(mario, simon);
				bricks[i]->updateFrame();
			}
			mario.paint(xinfo);
			mario.updateFrame();
			XFlush(xinfo.display);

			lastRepaint = now(); // remember when the paint happened
		}

		// IMPORTANT: sleep for a bit to let other processes work
		if (XPending(xinfo.display) == 0)
		{
			usleep(1000000 / FPS - (end - lastRepaint));
		}
	}
}

void computerPlay(Background &background, Mario &mario, MysteryBrick **bricks, Simon &simon)
{
	int n = simon.getNumButtons();

	// time of last window paint
	unsigned long startTime = now();
	unsigned long lastRepaint = startTime - 1000000 / FPS;

	// event loop
	while (simon.getState() == Simon::COMPUTER)
	{
		bricks[simon.nextButton()]->jump(true);
		while (true)
		{
			if (XPending(xinfo.display) > 0)
			{
				XNextEvent(xinfo.display, &xinfo.event);
				if (xinfo.event.type == KeyPress)
				{
					KeySym key;
					char text[10];
					int i = XLookupString((XKeyEvent *)&xinfo.event, text, 10, &key, 0);
					if (i == 1)
					{
						if (text[0] == 'q')
						{
							XFreePixmap(xinfo.display, xinfo.pixmap);
							for (int i = 0; i < n; ++i)
							{
								delete bricks[i];
							}
							XCloseDisplay(xinfo.display);
							exit(0);
						}
						else if (text[0] == ' ')
						{
							mario.jump();
						}
						else if (text[0] == 'a')
						{
							mario.runLeft();
						}
						else if (text[0] == 'd')
						{
							mario.runRight();
						}
					}
				}
				else if (xinfo.event.type == KeyRelease)
				{
					KeySym key;
					char text[10];
					int i = XLookupString((XKeyEvent *)&xinfo.event, text, 10, &key, 0);
					if (i == 1)
					{
						if (text[0] == 'a')
						{
							mario.runLeftStop();
						}
						else if (text[0] == 'd')
						{
							mario.runRightStop();
						}
					}
				}
				else if (xinfo.event.type == ConfigureNotify)
				{
				}
			}

			unsigned long end = now();

			if (end - lastRepaint > 1000000 / FPS)
			{
				// draw displayables
				background.paint(xinfo);
				for (int i = 0; i < n; ++i)
				{
					bricks[i]->paint(xinfo);
					bricks[i]->bumpedBy(mario, simon);
					bricks[i]->updateFrame();
				}
				mario.paint(xinfo);
				mario.updateFrame();
				XFlush(xinfo.display);

				lastRepaint = now(); // remember when the paint happened
				if (lastRepaint - startTime > 750000)
				{
					startTime = lastRepaint;
					break;
				}
			}

			// IMPORTANT: sleep for a bit to let other processes work
			if (XPending(xinfo.display) == 0)
			{
				usleep(1000000 / FPS - (end - lastRepaint));
			}
		}
	}
}

void playLoseAnimation(Background &background, Mario &mario, MysteryBrick **bricks, Simon &simon)
{
	int n = simon.getNumButtons();

	background.paint(xinfo);
	for (int i = 0; i < n; ++i)
	{
		bricks[i]->paint(xinfo);
		bricks[i]->updateFrame();
	}
	mario.paint(xinfo, Mario::LOSE);
	mario.jump(true);

	XFlush(xinfo.display);
	mario.updateFrame();

	//pause the shocked frame for a second
	usleep(500000);

	unsigned long startTime = now();
	unsigned long lastRepaint = startTime - 1000000 / FPS;
	while (true)
	{
		if (XPending(xinfo.display) > 0)
		{
			XNextEvent(xinfo.display, &xinfo.event);
			if (xinfo.event.type == KeyPress)
			{
				KeySym key;
				char text[10];
				int i = XLookupString((XKeyEvent *)&xinfo.event, text, 10, &key, 0);
				if (i == 1)
				{
					if (text[0] == 'q')
					{
						XFreePixmap(xinfo.display, xinfo.pixmap);
						for (int i = 0; i < n; ++i)
						{
							delete bricks[i];
						}
						XCloseDisplay(xinfo.display);
						exit(0);
					}
					else if (text[0] == ' ')
					{
						mario.jump();
					}
					else if (text[0] == 'a')
					{
						mario.runLeft();
					}
					else if (text[0] == 'd')
					{
						mario.runRight();
					}
				}
			}
			else if (xinfo.event.type == KeyRelease)
			{
				KeySym key;
				char text[10];
				int i = XLookupString((XKeyEvent *)&xinfo.event, text, 10, &key, 0);
				if (i == 1)
				{
					if (text[0] == 'a')
					{
						mario.runLeftStop();
					}
					else if (text[0] == 'd')
					{
						mario.runRightStop();
					}
				}
			}
			else if (xinfo.event.type == ConfigureNotify)
			{
			}
		}

		unsigned long end = now();

		if (end - lastRepaint > 1000000 / FPS)
		{
			// draw displayables
			background.paint(xinfo);
			for (int i = 0; i < n; ++i)
			{
				bricks[i]->paint(xinfo);
				bricks[i]->bumpedBy(mario, simon);
				bricks[i]->updateFrame();
			}
			mario.paint(xinfo, Mario::LOSE);
			mario.updateFrame();
			XFlush(xinfo.display);

			lastRepaint = now(); // remember when the paint happened
			if (lastRepaint - startTime > 800000)
			{
				startTime = lastRepaint;
				break;
			}
		}

		// IMPORTANT: sleep for a bit to let other processes work
		if (XPending(xinfo.display) == 0)
		{
			usleep(1000000 / FPS - (end - lastRepaint));
		}
	}
}

void playWinAnimation(Background &background, Mario &mario, MysteryBrick **bricks, Simon &simon)
{
	int n = simon.getNumButtons();

	unsigned long startTime = now();
	unsigned long lastRepaint = startTime - 1000000 / FPS;
	while (true)
	{
		if (XPending(xinfo.display) > 0)
		{
			XNextEvent(xinfo.display, &xinfo.event);
			if (xinfo.event.type == KeyPress)
			{
				KeySym key;
				char text[10];
				int i = XLookupString((XKeyEvent *)&xinfo.event, text, 10, &key, 0);
				if (i == 1)
				{
					if (text[0] == 'q')
					{
						XFreePixmap(xinfo.display, xinfo.pixmap);
						for (int i = 0; i < n; ++i)
						{
							delete bricks[i];
						}
						XCloseDisplay(xinfo.display);
						exit(0);
					}
					else if (text[0] == ' ')
					{
						mario.jump();
					}
					else if (text[0] == 'a')
					{
						mario.runLeft();
					}
					else if (text[0] == 'd')
					{
						mario.runRight();
					}
				}
			}
			else if (xinfo.event.type == KeyRelease)
			{
				KeySym key;
				char text[10];
				int i = XLookupString((XKeyEvent *)&xinfo.event, text, 10, &key, 0);
				if (i == 1)
				{
					if (text[0] == 'a')
					{
						mario.runLeftStop();
					}
					else if (text[0] == 'd')
					{
						mario.runRightStop();
					}
				}
			}
			else if (xinfo.event.type == ConfigureNotify)
			{
			}
		}

		unsigned long end = now();

		if (end - lastRepaint > 1000000 / FPS)
		{
			// draw displayables
			background.paint(xinfo);
			for (int i = 0; i < n; ++i)
			{
				bricks[i]->paint(xinfo);
				bricks[i]->bumpedBy(mario, simon);
				bricks[i]->updateFrame();
			}
			mario.paint(xinfo, Mario::WIN);
			mario.updateFrame();
			XFlush(xinfo.display);

			lastRepaint = now(); // remember when the paint happened
			if (lastRepaint - startTime > 400000)
			{
				startTime = lastRepaint;
				break;
			}
		}

		// IMPORTANT: sleep for a bit to let other processes work
		if (XPending(xinfo.display) == 0)
		{
			usleep(1000000 / FPS - (end - lastRepaint));
		}
	}
}

void userPlay(Background &background, Mario &mario, MysteryBrick **bricks, Simon &simon)
{
	int n = simon.getNumButtons();

	// time of last window paint
	unsigned long lastRepaint = 0;

	// event loop
	while (simon.getState() == Simon::HUMAN)
	{
		if (XPending(xinfo.display) > 0)
		{
			XNextEvent(xinfo.display, &xinfo.event);
			if (xinfo.event.type == KeyPress)
			{
				KeySym key;
				char text[10];
				int i = XLookupString((XKeyEvent *)&xinfo.event, text, 10, &key, 0);
				if (i == 1)
				{
					if (text[0] == 'q')
					{
						XFreePixmap(xinfo.display, xinfo.pixmap);
						for (int i = 0; i < n; ++i)
						{
							delete bricks[i];
						}
						XCloseDisplay(xinfo.display);
						exit(0);
					}
					else if (text[0] == ' ')
					{
						mario.jump();
					}
					else if (text[0] == 'a')
					{
						mario.runLeft();
					}
					else if (text[0] == 'd')
					{
						mario.runRight();
					}
				}
			}
			else if (xinfo.event.type == KeyRelease)
			{
				KeySym key;
				char text[10];
				int i = XLookupString((XKeyEvent *)&xinfo.event, text, 10, &key, 0);
				if (i == 1)
				{
					if (text[0] == 'a')
					{
						mario.runLeftStop();
					}
					else if (text[0] == 'd')
					{
						mario.runRightStop();
					}
				}
			}
			else if (xinfo.event.type == ConfigureNotify)
			{
			}
		}

		unsigned long end = now();

		if (end - lastRepaint > 1000000 / FPS)
		{
			// draw displayables
			background.paint(xinfo);
			for (int i = 0; i < n; ++i)
			{
				bricks[i]->paint(xinfo);
				bricks[i]->bumpedBy(mario, simon, true);
				bricks[i]->updateFrame();
			}
			mario.paint(xinfo);
			mario.updateFrame();
			XFlush(xinfo.display);

			lastRepaint = now(); // remember when the paint happened
		}

		// IMPORTANT: sleep for a bit to let other processes work
		if (XPending(xinfo.display) == 0)
		{
			usleep(1000000 / FPS - (end - lastRepaint));
		}
	}
}

int main(int argc, char *argv[])
{
	// get the number of buttons from args
	// (default to 4 if no args)
	int n = 4;
	if (argc > 1)
	{
		n = atoi(argv[1]);
	}
	n = max(1, min(n, 6));

	initX(xinfo);

	int d = 20;
	int distance = (xinfo.w.width - n * d) / (n + 1);
	int y = xinfo.w.height / 2;
	int x = distance + d / 2;
	MysteryBrick *bricks[n];
	for (int i = 0; i < n; ++i)
	{
		bricks[i] = new MysteryBrick(x, y, xinfo, i);
		x += distance + d;
	}
	Mario mario = Mario{100, y + 150, xinfo};
	Background background = Background{mario.getY() + 12 * 2, xinfo};

	Simon simon = Simon(n);

	while (true)
	{
		switch (simon.getState())
		{

		// will only be in this state right after Simon object is contructed
		case Simon::START:
			break;
		// score is increased by 1, sequence length is increased by 1
		case Simon::WIN:
			playWinAnimation(background, mario, bricks, simon);
			break;
		// score is reset to 0, sequence length is reset to 1
		case Simon::LOSE:
			playLoseAnimation(background, mario, bricks, simon);
			break;
		default:
			break;
		}
		waitforE(background, mario, bricks, simon);

		// start new round with a new sequence
		simon.newRound();
		// computer plays
		computerPlay(background, mario, bricks, simon);
		userPlay(background, mario, bricks, simon);
	}
}