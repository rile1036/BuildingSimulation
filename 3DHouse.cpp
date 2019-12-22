
#include "3Dbuild.h"

// 메모리에 TGA 파일을 로드하는 함수
bool load_TGA(TextureImage *texture, char *filename)
{
	GLubyte		TGAheader[12] = { 0,0,2,0,0,0,0,0,0,0,0,0 };					// Uncompressed TGA Header
	GLubyte		TGAcompare[12];												// Used To Compare TGA Header
	GLubyte		header[6];													// First 6 Useful Bytes From The Header
	GLuint		bytesPerPixel;												// Holds Number Of Bytes Per Pixel Used In The TGA File
	GLuint		imageSize;													// Used To Store The Image Size When Setting Aside Ram
	GLuint		temp;														// Temporary Variable
	GLuint		type = GL_RGBA;												// Set The Default GL Mode To RBGA (32 BPP)

	FILE *file = fopen(filename, "rb");										// Open The TGA File

	if (file == NULL ||														// Does File Even Exist?
		fread(TGAcompare, 1, sizeof(TGAcompare), file) != sizeof(TGAcompare) ||	// Are There 12 Bytes To Read?
		memcmp(TGAheader, TGAcompare, sizeof(TGAheader)) != 0 ||	// Does The Header Match What We Want?
		fread(header, 1, sizeof(header), file) != sizeof(header))				// If So Read Next 6 Header Bytes
	{
		if (file == NULL)													// Did The File Even Exist? *Added Jim Strong*
			return FALSE;													// Return False
		else																// Otherwise
		{
			fclose(file);													// If Anything Failed, Close The File
			return FALSE;													// Return False
		}
	}

	texture->width = header[1] * 256 + header[0];							// Determine The TGA Width	(highbyte*256+lowbyte)
	texture->height = header[3] * 256 + header[2];							// Determine The TGA Height	(highbyte*256+lowbyte)

	if (texture->width <= 0 ||												// Is The Width Less Than Or Equal To Zero
		texture->height <= 0 ||												// Is The Height Less Than Or Equal To Zero
		(header[4] != 24 && header[4] != 32))									// Is The TGA 24 or 32 Bit?
	{
		fclose(file);														// If Anything Failed, Close The File
		return FALSE;														// Return False
	}

	texture->bpp = header[4];											// Grab The TGA's Bits Per Pixel (24 or 32)
	bytesPerPixel = texture->bpp / 8;										// Divide By 8 To Get The Bytes Per Pixel
	imageSize = texture->width*texture->height*bytesPerPixel;			// Calculate The Memory Required For The TGA Data

	texture->imageData = (GLubyte *)malloc(imageSize);						// Reserve Memory To Hold The TGA Data

	if (texture->imageData == NULL ||											// Does The Storage Memory Exist?
		fread(texture->imageData, 1, imageSize, file) != imageSize)			// Does The Image Size Match The Memory Reserved?
	{
		if (texture->imageData != NULL)										// Was Image Data Loaded
			free(texture->imageData);										// If So, Release The Image Data

		fclose(file);														// Close The File
		return FALSE;														// Return False
	}

	for (GLuint i = 0; i<int(imageSize); i += bytesPerPixel)						// Loop Through The Image Data
	{																		// Swaps The 1st And 3rd Bytes ('R'ed and 'B'lue)
		temp = texture->imageData[i];											// Temporarily Store The Value At Image Data 'i'
		texture->imageData[i] = texture->imageData[i + 2];					// Set The 1st Byte To The Value Of The 3rd Byte
		texture->imageData[i + 2] = temp;									// Set The 3rd Byte To The Value In 'temp' (1st Byte Value)
	}

	fclose(file);															// Close The File

																			// Build A Texture From The Data
	glGenTextures(1, &texture[0].texID);									// Generate OpenGL texture IDs
	glBindTexture(GL_TEXTURE_2D, texture[0].texID);							// Bind Our Texture
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);		// Linear Filtered
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);		// Linear Filtered
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	if (texture[0].bpp == 24)													// Was The TGA 24 Bits
	{
		type = GL_RGB;														// If So Set The 'type' To GL_RGB
	}

	glTexImage2D(GL_TEXTURE_2D, 0, type, texture[0].width, texture[0].height, 0, type, GL_UNSIGNED_BYTE, texture[0].imageData);

	return true;															// Texture Building Went Ok, Return True
}

// 텍스트 파일에서 모든 값을 읽어들여와 모든 모델링을 설정하는 함수
void setup_world()
{
	float x, y, z, u, v;
	int numpolygons; // 폴리곤 총 개수
	int texid; // 텍스쳐 아이디
	FILE *filein;
	char oneline[255]; // 전체 라인을 읽어올 변수
	filein = fopen("Data/world.txt", "rt");		// data/world.txt라는 파일을 연다.

	readstr(filein, oneline); // 한 라인을 읽어온다.
	sscanf(oneline, "NUMPOLLIES %d\n", &numpolygons); // 첫 라인은 폴리곤 총 개수를 정의해놓은 라인, 여기서 폴리곤 총 개수 값을 가져와 지역 변수에 저장
	g_sector1.polygon = new POLYGON[numpolygons]; // 폴리곤 총 개수만큼 동적할당
	g_sector1.texture = new int[numpolygons];	 // 폴리곤 총 개수만큼 동적할당
	g_sector1.numpolygons = numpolygons; // 지역변수에 있던 값(폴리곤 총 개수)을 다시 전역 변수에 저장								

										 // 폴리곤 총 개수만큼 반복
	for (int loop = 0; loop < numpolygons; loop++)
	{
		readstr(filein, oneline); // 한 라인을 읽어온다.
		sscanf(oneline, "TEXTURE %d\n", &texid);	// 지역변수에 텍스쳐 아이디 저장
		g_sector1.texture[loop] = texid; // 저장된 텍스쳐 아이디를 변수에 다시 저장
		for (int vert = 0; vert < 4; vert++)
		{
			readstr(filein, oneline);	// 한 라인을 읽어온다
			sscanf(oneline, "%f %f %f %f %f %f %f", &x, &y, &z, &u, &v); // 저장된 순서대로 x, y, z, u, v를 순서대로 가져와 지역 변수에 저장
			g_sector1.polygon[loop].vertex[vert].x = x;
			g_sector1.polygon[loop].vertex[vert].y = y;
			g_sector1.polygon[loop].vertex[vert].z = z;
			g_sector1.polygon[loop].vertex[vert].u = u;
			g_sector1.polygon[loop].vertex[vert].v = v; // 다시 지역변수에 저장된 값을 다시 전역 변수에 저장
		}
	}
	fclose(filein); // 끝나면 파일을 닫는다.
	return;
}

// 텍스트 파일에서 한 라인씩 읽어오기 위한 함수
void readstr(FILE* f, char* string)
{
	do
	{
		fgets(string, 255, f);
	} while ((string[0] == '/') || (string[0] == '\n'));
}

// 텍스쳐 세팅 함수
bool setup_textures()
{
	// 어느 하나라도 로딩이 실패했을 경우
	if (!load_TGA(&textures[0], "Data/floor.tga")
		|| !load_TGA(&textures[1], "Data/tiled.tga")
		|| !load_TGA(&textures[2], "Data/painting1.tga")
		|| !load_TGA(&textures[3], "Data/painting2.tga")
		|| !load_TGA(&textures[4], "Data/painting3.tga")
		|| !load_TGA(&textures[5], "Data/painting4.tga")
		|| !load_TGA(&textures[7], "Data/painting6.tga")
		|| !load_TGA(&textures[6], "Data/painting5.tga")
		|| !load_TGA(&textures[8], "Data/painting8.tga")
		|| !load_TGA(&textures[9], "Data/painting8.tga")
		|| !load_TGA(&textures[10], "Data/painting9.tga")
		|| !load_TGA(&textures[11], "Data/graywall.tga") // 회색 벽
		|| !load_TGA(&textures[12], "Data/door.tga")
		|| !load_TGA(&textures[13], "Data/crosshairs.tga")
		|| !load_TGA(&textures[14], "Data/hardwood.tga") // 나무
		|| !load_TGA(&textures[15], "Data/stairsbottom.tga") // 
		|| !load_TGA(&textures[16], "Data/atticceiling.tga")
		|| !load_TGA(&textures[17], "Data/elevatordoor.tga") // 엘리베이터 문
		|| !load_TGA(&textures[18], "Data/railing.tga")
		|| !load_TGA(&textures[19], "Data/font.tga")
		|| !load_TGA(&textures[20], "Data/opendoor.tga") // 문틀
		|| !load_TGA(&textures[21], "Data/simpledoor.tga") // 문
		|| !load_TGA(&textures[22], "Data/column.tga")
		|| !load_TGA(&textures[23], "Data/screen.tga")
		|| !load_TGA(&textures[24], "Data/whiteboard.tga")
		|| !load_TGA(&textures[25], "Data/monitor.tga")
		|| !load_TGA(&textures[26], "Data/mirror.tga"))
		return false; // 실패했음을 리턴
	return true; // 모든 텍스쳐 파일이 올바르게 로드되었다면 성공했음을 리턴
}

// 폰트 텍스쳐 설정
GLvoid build_front(GLvoid)
{
	base = glGenLists(95);											// Creating 95 Display Lists
	glBindTexture(GL_TEXTURE_2D, textures[19].texID);				// Bind Our Font Texture
	for (int loop = 0; loop<95; loop++)								// Loop Through All 95 Lists
	{
		float cx = float(loop % 16) / 16.0f;								// X Position Of Current Character
		float cy = float(loop / 16) / 8.0f;								// Y Position Of Current Character

		glNewList(base + loop, GL_COMPILE);							// Start Building A List
		glBegin(GL_QUADS);											// Use A Quad For Each Character
		glTexCoord2f(cx, 1.0f - cy - 0.120f); glVertex2i(0, 0);	// Texture / Vertex Coord (Bottom Left)
		glTexCoord2f(cx + 0.0625f, 1.0f - cy - 0.120f); glVertex2i(16, 0);	// Texutre / Vertex Coord (Bottom Right)
		glTexCoord2f(cx + 0.0625f, 1.0f - cy);		  glVertex2i(16, 16);// Texture / Vertex Coord (Top Right)
		glTexCoord2f(cx, 1.0f - cy);		  glVertex2i(0, 16);	// Texture / Vertex Coord (Top Left)
		glEnd();													// Done Building Our Quad (Character)
		glTranslated(15, 0, 0);										// Move To The Right Of The Character
		glEndList();												// Done Building The Display List
	}																// Loop Until All 256 Are Built
}

// 초기화
bool init()
{
	glEnable(GL_LIGHT0);											// 0번 광원 활성화
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, GlobalAmbient);			// 전역 조명 모델 설정

	setup_textures();												// 텍스쳐 세팅
	build_front();													// 폰트 텍스쳐 설정
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);							// Pixel Storage Mode To Byte Alignment
	glEnable(GL_TEXTURE_2D);										// 텍스쳐 매핑 활성화
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);							// 배경색 초기화
	glClearDepth(1.0f);												// 깊이 설정
	glDepthFunc(GL_LESS);											// 깊이 버퍼 처리 설정
	glEnable(GL_DEPTH_TEST);										// 깊이 버퍼 활성화
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);				// Alpha blending 설정
	glEnable(GL_BLEND);												// blending 활성화
	glAlphaFunc(GL_GREATER, 0.1f);
	glEnable(GL_ALPHA_TEST);
	glShadeModel(GL_SMOOTH);										// 고러드 음영 모델 사용
	setup_world();													// 텍스트 파일에서 모든 값을 읽어들여와 모든 모델링을 설정한다.
	memset(g_key, 0, sizeof(g_key));
	return true;
}

// 디스플레이 재구성 이벤트
void reshape(int w, int h)
{
	window_width = w;
	window_height = h;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);									// 투영행렬 선택
	glLoadIdentity();												// 항등행렬로 초기화
	if (h == 0)
		h = 1;
	gluPerspective(45.0f, (float)w / (float)h, 0.1, 100.0);			// 원근 투영
	glMatrixMode(GL_MODELVIEW);										// 모델뷰행렬 선택
	glLoadIdentity();												// 항등행렬로 초기화
}

// 폰트 출력
GLvoid gl_print(GLint x, GLint y, const char *string, ...)
{
	char text[256];	 // 넘겨온 string 값을 저장하는 변수
	va_list	ap;		// Pointer To List Of Arguments

					// string 값이 없다면
	if (string == NULL)
		return;	// 아무 것도 하지 않는다.

	va_start(ap, string);
	vsprintf(text, string, ap);
	va_end(ap);

	// 읽어들인 string 값을 텍스쳐와 매핑하여
	// 화면에 텍스쳐 매핑한 물체로써 출력한다.
	glBindTexture(GL_TEXTURE_2D, textures[19].texID);
	glPushMatrix();
	glLoadIdentity();
	glTranslated(x, y, 0);
	glListBase(base - 32);
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);
	glPopMatrix();
}

// 텍스쳐 오브젝트를 생성하는 함수
// world.txt에서 읽어들여온 vertex (x, y, z, u,v) 값을 이용하여 텍스쳐 오브젝트 생성
void texture_object(int polygonNum)
{
	GLfloat x_m, y_m, z_m, u_m, v_m;

	glBindTexture(GL_TEXTURE_2D, textures[g_sector1.texture[polygonNum]].texID);
	glBegin(GL_POLYGON);
	glNormal3f(0.0f, 0.0f, 1.0f); // 법선백터 사용
	x_m = g_sector1.polygon[polygonNum].vertex[0].x;
	y_m = g_sector1.polygon[polygonNum].vertex[0].y;
	z_m = g_sector1.polygon[polygonNum].vertex[0].z;
	u_m = g_sector1.polygon[polygonNum].vertex[0].u;
	v_m = g_sector1.polygon[polygonNum].vertex[0].v;
	glTexCoord2f(u_m, v_m); glVertex3f(x_m, y_m, z_m);

	x_m = g_sector1.polygon[polygonNum].vertex[1].x;
	y_m = g_sector1.polygon[polygonNum].vertex[1].y;
	z_m = g_sector1.polygon[polygonNum].vertex[1].z;
	u_m = g_sector1.polygon[polygonNum].vertex[1].u;
	v_m = g_sector1.polygon[polygonNum].vertex[1].v;
	glTexCoord2f(u_m, v_m); glVertex3f(x_m, y_m, z_m);

	x_m = g_sector1.polygon[polygonNum].vertex[2].x;
	y_m = g_sector1.polygon[polygonNum].vertex[2].y;
	z_m = g_sector1.polygon[polygonNum].vertex[2].z;
	u_m = g_sector1.polygon[polygonNum].vertex[2].u;
	v_m = g_sector1.polygon[polygonNum].vertex[2].v;
	glTexCoord2f(u_m, v_m); glVertex3f(x_m, y_m, z_m);

	x_m = g_sector1.polygon[polygonNum].vertex[3].x;
	y_m = g_sector1.polygon[polygonNum].vertex[3].y;
	z_m = g_sector1.polygon[polygonNum].vertex[3].z;
	u_m = g_sector1.polygon[polygonNum].vertex[3].u;
	v_m = g_sector1.polygon[polygonNum].vertex[3].v;
	glTexCoord2f(u_m, v_m); glVertex3f(x_m, y_m, z_m);
	glEnd();
}

// 디스플레이 이벤트
void render(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	 // 색상 버퍼 및 깊이 버퍼 초기화
	glLoadIdentity();	// 모델뷰 행렬 초기화

	GLfloat xtrans = -g_xpos;
	GLfloat ztrans = -g_zpos;
	GLfloat ytrans = -g_ypos;
	if (g_yrot > 360)
		g_yrot -= 360;
	else if (g_yrot < 0)
		g_yrot += 360;
	GLfloat sceneroty = (360.0f - g_yrot);

	// 0번 광원이 바라보는 방향
	GLfloat round_x = sin(g_yrot * PI_OVER_180 / 180);
	GLfloat round_z = -cos(g_yrot * PI_OVER_180 / 180);

	GLfloat Light0_Ambient[] = { 1.0, 1.0, 1.0, 1.0 }; //0번 광원 특성
	GLfloat Light0_Diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat Light0_Specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat Light0_Position[] = { g_xpos, g_ypos, g_zpos + 1, 1.0 }; //0번 광원 위치
	GLfloat Light0_Direction[] = { round_x, 0.0, round_z, 0.0 }; //0번 광원 방향
	GLfloat Light0_Cutoff[] = { 80.0 }; //0번 광원 각도

	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.5);
	glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 1.0);
	glLightfv(GL_LIGHT0, GL_AMBIENT, Light0_Ambient); //0번 광원 특성할당
	glLightfv(GL_LIGHT0, GL_DIFFUSE, Light0_Diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, Light0_Specular);
	glLightfv(GL_LIGHT0, GL_POSITION, Light0_Position); // 0번 광원 위치 할당
	glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, Light0_Direction); // 스포트라이트 방향 할당
	glLightfv(GL_LIGHT0, GL_SPOT_CUTOFF, Light0_Cutoff); // 스포트라이트 각도 할당

	int numpolygons;

	glRotatef(g_lookupdown, 1.0f, 0, 0);
	glRotatef(sceneroty, 0, 1.0f, 0);

	glTranslatef(xtrans, ytrans, ztrans);

	numpolygons = g_sector1.numpolygons; // 폴리곤의 총 개수를 가져온다.

										 // 폴리곤의 총 개수만큼 반복
	for (int loop_m = 0; loop_m < numpolygons; loop_m++)
		texture_object(loop_m);	// 텍스쳐 오브젝트를 생성한다.

	glMatrixMode(GL_PROJECTION); // 투영행렬 선택					
	glPushMatrix();	// 투영행렬에 현재 투영행렬값 저장				
	glLoadIdentity(); // 투영행렬 초기화								
	glOrtho(-10, window_width, 0, window_height, -10, 10);// 직교 투영 실시

	glMatrixMode(GL_MODELVIEW);	// 모델뷰 행렬 선택

								// 화면상에 X,Y,Z 좌표를 출력
	glDisable(GL_LIGHTING);
	glEnable(GL_LIGHTING);

	// ENTER 키가 눌린 상태라면
	// 키보드로 입력할 수 있는 키들의 기능들을 알려준다 
	if (ENTER_WAS_PRESSED)
	{
		glDisable(GL_LIGHTING);
		glDisable(GL_BLEND);
		glDisable(GL_ALPHA_TEST);
		gl_print((window_width / 2) - 200, (window_height / 2) + 80, "Up Arrow: Move Forward");
		gl_print((window_width / 2) - 200, (window_height / 2) + 60, "Down Arrow: Move Backwards");
		gl_print((window_width / 2) - 200, (window_height / 2) + 40, "Left Arrow: Turn Left");
		gl_print((window_width / 2) - 200, (window_height / 2) + 20, "Right Arrow: Turn Right");
		gl_print((window_width / 2) - 200, (window_height / 2), "Page Up: Look up");
		gl_print((window_width / 2) - 200, (window_height / 2) - 20, "Page Down: Look Down");
		gl_print((window_width / 2) - 200, (window_height / 2) - 40, "Spacebar: Jump");
		gl_print((window_width / 2) - 200, (window_height / 2) - 60, "Plus Sign: Speed Up Movement");
		gl_print((window_width / 2) - 200, (window_height / 2) - 80, "Minus Sign: Slow Down Movement");
		gl_print((window_width / 2) - 200, (window_height / 2) - 100, "Mouse Left button : Light Side");
		gl_print((window_width / 2) - 200, (window_height / 2) - 120, "Mouse Right button : Light Off");
		glEnable(GL_LIGHTING);
		glEnable(GL_BLEND);
		glEnable(GL_ALPHA_TEST);
	}

	// 만약 열 수 있는 문의 위치에 가깝다면
	if (g_zpos < 0.0 && g_zpos > -3.3 && g_xpos < 3.0 && g_xpos > 2.0 && g_ypos < 0.5) {
		glDisable(GL_LIGHTING);
		glEnable(GL_LIGHTING);
	}


	glMatrixMode(GL_PROJECTION); // 투영행렬 선택
	glPopMatrix(); // 저장된 행렬값 복구
	glMatrixMode(GL_MODELVIEW);	// 모델뷰행렬 선택

								// 조명 활성화 상태값에 의해
								// 조명 활성화 또는 비활성화
	if (LIGHT_ON == true)
		glEnable(GL_LIGHTING);
	else
		glDisable(GL_LIGHTING);

	glutSwapBuffers(); // 출력
}

// 문을 여는 함수
// 키보드 이벤트에서 사용
// 문이 존재하는 위치에서 이벤트 사용 가능
void openDoor()
{
	if (!DOOR_IS_OPEN) // 문의 상태가 열려있지 않은 상태라면
	{
		DOOR_IS_OPEN = true; // 문의 상태를 연 상태로 바꾸고
							 // 110번 반복하여 문이 열리는 애니메이션을 만든다.
		for (int i = 0; i < 110; i++)
		{
			g_sector1.polygon[0].vertex[0].x = 2.13 + (0.373 - (0.373 * cos(i*PI_OVER_180)));
			g_sector1.polygon[0].vertex[1].x = 2.13 + (0.373 - (0.373 * cos(i*PI_OVER_180)));
			g_sector1.polygon[0].vertex[0].z = -1.999 - (0.373 - (0.373 * cos(i*PI_OVER_180)));
			g_sector1.polygon[0].vertex[1].z = -1.999 - (0.373 - (0.373 * cos(i*PI_OVER_180)));
			Sleep(7); // 문이 열리는 동안 움직이지 못하도록 슬립을 건다.
			render(); // 디스플레이 이벤트 호출
		}
	}
	else // 문의 상태가 열려있는 상태라면
	{
		DOOR_IS_OPEN = false; // 문의 상태를 열려있지 않은 상태로 바꾸고
							  // 110번 디스플레이 이벤트를 호출하여 문이 닫히는 애니메이션을 만든다.
		for (int i = 0; i < 110; i++)
		{
			g_sector1.polygon[0].vertex[0].x = 2.61 - (0.373 - (0.373 * cos(i*PI_OVER_180)));
			g_sector1.polygon[0].vertex[1].x = 2.61 - (0.373 - (0.373 * cos(i*PI_OVER_180)));
			g_sector1.polygon[0].vertex[0].z = -2.49 + (0.373 - (0.373 * cos(i*PI_OVER_180)));
			g_sector1.polygon[0].vertex[1].z = -2.49 + (0.373 - (0.373 * cos(i*PI_OVER_180)));
			Sleep(7); // 문이 열리는 동안 움직이지 못하도록 슬립을 건다.
			render(); // 디스플레이 이벤트 호출
		}
	}
}

void openDoor_1()
{
	if (!DOOR_IS_OPEN) // 문의 상태가 열려있지 않은 상태라면
	{
		DOOR_IS_OPEN = true; // 문의 상태를 연 상태로 바꾸고
							 // 110번 반복하여 문이 열리는 애니메이션을 만든다.
		for (int i = 0; i < 110; i++)
		{
			g_sector1.polygon[0].vertex[0].x = 8.29 + (0.373 - (0.373 * cos(i*PI_OVER_180)));
			g_sector1.polygon[0].vertex[1].x = 8.29 + (0.373 - (0.373 * cos(i*PI_OVER_180)));
			g_sector1.polygon[0].vertex[0].z = -2.99 - (0.373 - (0.373 * cos(i*PI_OVER_180)));
			g_sector1.polygon[0].vertex[1].z = -2.99 - (0.373 - (0.373 * cos(i*PI_OVER_180)));
			Sleep(7); // 문이 열리는 동안 움직이지 못하도록 슬립을 건다.
			render(); // 디스플레이 이벤트 호출
		}
	}
	else // 문의 상태가 열려있는 상태라면
	{
		DOOR_IS_OPEN = false; // 문의 상태를 열려있지 않은 상태로 바꾸고
							  // 110번 디스플레이 이벤트를 호출하여 문이 닫히는 애니메이션을 만든다.
		for (int i = 0; i < 110; i++)
		{
			g_sector1.polygon[0].vertex[0].x = 8.77 - (0.373 - (0.373 * cos(i*PI_OVER_180)));
			g_sector1.polygon[0].vertex[1].x = 8.77 - (0.373 - (0.373 * cos(i*PI_OVER_180)));
			g_sector1.polygon[0].vertex[0].z = -3.481 + (0.373 - (0.373 * cos(i*PI_OVER_180)));
			g_sector1.polygon[0].vertex[1].z = -3.481 + (0.373 - (0.373 * cos(i*PI_OVER_180)));
			Sleep(7); // 문이 열리는 동안 움직이지 못하도록 슬립을 건다.
			render(); // 디스플레이 이벤트 호출
		}
	}
}

// 키보드 이벤트
void keyboard(unsigned char key, int x, int y)
{
	GLfloat jumpTop = g_ypos + 0.25; // 점프로 올라갈 수 있는 최대 y값
	GLfloat jumpBottom = g_ypos + (speed_XYZ / 2); // 점프 후 내려갈 수 있는 최소 y값
	switch (key)
	{
	case ' ': // 스페이스 키가 눌렸다면 점프
		if (g_zpos < -4.5 && g_xpos < 0.2 && g_xpos > -0.2)
			jumpBottom = 0.45 + (speed_XYZ / 2);
		g_ypos += (speed_XYZ / 4);
		while (g_ypos < jumpTop) // 점프 top보다 y값이 작으면
		{
			g_ypos += (speed_XYZ / 2); // y 값을 계속 증가
			glutPostRedisplay(); // 디스플레이 이벤트 강제 호출
		}
		// 점프 최대까지 올라온 상태
		while (g_ypos > jumpBottom) // 점프 bottom보다 y값이 크면 
		{
			g_ypos -= (speed_XYZ / 2); // y 값을 계속 감소
			glutPostRedisplay(); // 디스플레이 이벤트 강제 호출
		}
		g_ypos -= (speed_XYZ / 4);
		// 다시 원래 y값으로 복귀
		break;
	case 'x': // x 또는
	case 'X': // X가 입력되었을 경우
			  // 문이 있는 위치라면
		if (g_zpos < 0.0 && g_zpos > -3.3 && g_xpos < 3.0 && g_xpos > 2.0 && g_ypos < 0.5) {
			openDoor(); // 문을 열거나 닫는다.
		}
		else if (g_zpos < -2.0 && g_zpos > -4.0 && g_xpos < 9.0 && g_xpos > 8.0 && g_ypos < 0.5) {
			openDoor_1();
		}
		break;
	case '-': // -가 입력되면
			  // 이동속도를 감소
		if (speed_UDLR > 0.02 && speed_XYZ > 0.002)
		{
			speed_UDLR -= 0.01;
			speed_XYZ -= 0.001;
		}
		break;
	case '+': // +가 입력되면
			  // 이동속도를 증가
		speed_UDLR += 0.01;
		speed_XYZ += 0.001;
		break;
	case 13: // ENTER가 눌렸다면
			 // 누른 상태가 아니라면
		if (!ENTER_WAS_PRESSED)
			ENTER_WAS_PRESSED = true; // 변수를 누른 상태로 변화
		else // 누른 상태가 맞다면
			ENTER_WAS_PRESSED = false; // 변수를 누르지 않은 상태로 변화
		break;
	case 27: // esc가 눌렸다면
		exit(0); // 시스템 종료
		break;
	default:
		break;
	}
	render();
}

// 특수키 이벤트
// 키가 눌렸다면 키의 상태값을 true, 눌려진 상태로 한다.
void special_keys(int a_keys, int x, int y)
{
	switch (a_keys)
	{
	case GLUT_KEY_F1:
		if (!g_gamemode)
		{
			g_fullscreen = !g_fullscreen;						// Toggle g_fullscreen Flag
			if (g_fullscreen) glutFullScreen();					// We Went In Fullscreen Mode
			else glutReshapeWindow(window_width, window_height);  // We Went In Windowed Mode
		}
		break;
	default:
		g_key[a_keys] = true;
		break;
	}
}

// 특수키가 누른 상태에서 눌러지지 않은 상태로 변화했을 때 호출되는 이벤트
void special_keys_up(int key, int x, int y)
{
	g_key[key] = false; // 그 키의 현재 상태 값을 false, 즉 눌러지지 않은 상태로 한다.
}

// idle 이벤트로 등록된 game_function
// 현재 키의 상태값에 의해 전역변수를 변경하여
// 캐릭터의 움직임을 결정한다.
void game_function()
{
	// page up이 눌렸다면
	// 시점을 위로 변경한다.
	if (g_key[GLUT_KEY_PAGE_UP])
	{
		g_z -= 0.002f;
		g_lookupdown -= speed_UDLR;
	}
	// page down이 눌렸다면
	// 시점을 아래로 한다..
	if (g_key[GLUT_KEY_PAGE_DOWN])
	{
		g_z += 0.002f;
		g_lookupdown += speed_UDLR;
	}
	// 위쪽 방향키가 눌렸다면
	// 전진한다.
	if (g_key[GLUT_KEY_UP])
	{
		// 전진하는 곳이 계단 부분이라면
		// 캐릭터의 y축 값을 계단에 맞춰서 감소하거나 증가시킨다. 즉, 오르락내리락한다.
		if (g_zpos > 0.0 && g_zpos < 0.32 && g_xpos > 3.3 && g_xpos < 3.6 && g_ypos < 0.47 && g_ypos >= -0.27)
		{
			g_xpos -= (float)sin(g_yrot*PI_OVER_180) * (speed_XYZ / 2);
			g_zpos -= (float)cos(g_yrot*PI_OVER_180) * (speed_XYZ / 2);
			if (g_ypos > -0.27 && (g_yrot <= 200 && g_yrot >= 160))
				g_ypos -= ((3 * speed_XYZ / 5) + 0.006);
		}
		else if (g_zpos > 0.0 && g_zpos < 0.7 && g_xpos > 3.3 && g_xpos < 3.6 && g_ypos < 0.45 && g_ypos >= -0.57)
		{
			g_xpos -= (float)sin(g_yrot*PI_OVER_180) * (speed_XYZ / 2);
			g_zpos -= (float)cos(g_yrot*PI_OVER_180) * (speed_XYZ / 2);
			if (g_ypos > -0.57 && (g_yrot >= 270 || g_yrot <= 90))
				g_ypos += ((3 * speed_XYZ / 5) + 0.006);
		}
		// 그렇지 않다면
		// 캐릭터가 바라보는 방향으로 전진한다.
		else {
			g_xpos -= (float)sin(g_yrot*PI_OVER_180) * speed_XYZ;
			g_zpos -= (float)cos(g_yrot*PI_OVER_180) * speed_XYZ;

			// 충돌이 존재한다면
			// 전진을 막는다.
			if (!no_collision())
			{
				g_xpos += (float)sin(g_yrot*PI_OVER_180) * speed_XYZ;
				g_zpos += (float)cos(g_yrot*PI_OVER_180) * speed_XYZ;
			}
		}
	}
	// 아래쪽 방향키가 눌렸다면
	// 후진한다.
	if (g_key[GLUT_KEY_DOWN])
	{
		// 후진하는 곳이 계단 부분이라면
		// 캐릭터의 y축 값을 계단에 맞춰서 감소하거나 증가시킨다. 즉, 오르락내리락한다.
		if (g_zpos > 0.0 && g_zpos < 0.32 && g_xpos > 3.3 && g_xpos < 3.6 && g_ypos < 0.47 && g_ypos >= -0.27)
		{
			g_xpos -= (float)sin(g_yrot*PI_OVER_180) * (speed_XYZ / 2);
			g_zpos -= (float)cos(g_yrot*PI_OVER_180) * (speed_XYZ / 2);
			if (g_ypos > -0.27 && (g_yrot <= 200 && g_yrot >= 160))
				g_ypos -= ((3 * speed_XYZ / 5) + 0.006);
		}
		else if (g_zpos > 0.0 && g_zpos < 0.7 && g_xpos > 3.3 && g_xpos < 3.6 && g_ypos < 0.45 && g_ypos >= -0.57)
		{
			g_xpos -= (float)sin(g_yrot*PI_OVER_180) * (speed_XYZ / 2);
			g_zpos -= (float)cos(g_yrot*PI_OVER_180) * (speed_XYZ / 2);
			if (g_ypos > -0.57 && (g_yrot >= 270 || g_yrot <= 90))
				g_ypos += ((3 * speed_XYZ / 5) + 0.006);
		}
		else if (g_zpos > -0.1 && g_zpos < 0.32 && g_xpos > 3.8 && g_xpos < 4.4 && g_ypos < -0.15 && g_ypos >= -0.67)
		{
			g_xpos += (float)sin(g_yrot*PI_OVER_180) * (speed_XYZ / 2);
			g_zpos += (float)cos(g_yrot*PI_OVER_180) * (speed_XYZ / 2);
			if (g_ypos > -0.97 && (g_yrot >= 270 || g_yrot <= 90))
				g_ypos -= ((3 * speed_XYZ / 5) + 0.006);
		}
		else if (g_zpos > -0.1 && g_zpos < 0.7 && g_xpos > 3.8 && g_xpos < 4.4 && g_ypos < -0.15 && g_ypos >= -0.97)
		{
			g_xpos += (float)sin(g_yrot*PI_OVER_180) * (speed_XYZ / 2);
			g_zpos += (float)cos(g_yrot*PI_OVER_180) * (speed_XYZ / 2);
			if (g_ypos > -0.97 && (g_yrot <= 200 || g_yrot >= 160))
				g_ypos += ((3 * speed_XYZ / 5) + 0.006);
		}
		// 그렇지 않다면
		// 캐릭터가 바라보는 반대 방향으로 후진한다.
		else
		{
			g_xpos += (float)sin(g_yrot*PI_OVER_180) * speed_XYZ;
			g_zpos += (float)cos(g_yrot*PI_OVER_180) * speed_XYZ;

			// 충돌이 존재한다면
			// 후진을 막는다.
			if (!no_collision())
			{
				g_xpos -= (float)sin(g_yrot*PI_OVER_180) * speed_XYZ;
				g_zpos -= (float)cos(g_yrot*PI_OVER_180) * speed_XYZ;
			}
		}
	}
	// 오른쪽 방향키가 눌렸다면
	// 시점을 우로 변경한다.
	if (g_key[GLUT_KEY_RIGHT])
		g_yrot -= speed_UDLR;
	// 왼쪽 방향키가 눌렸다면
	// 시점을 좌로 변경한다.
	if (g_key[GLUT_KEY_LEFT])
		g_yrot += speed_UDLR;
	//Sleep(1);
	glutPostRedisplay(); // 디스플레이 이벤트 강제 호출
}

// 충돌을 감지하는 함수
bool no_collision()
{
	return true;
}

// 마우스 클릭 이벤트
// 왼쪽 버튼을 누르면 조명 활성화
// 오른쪽 버튼을 누르면 조명 비활성화
void mouse_click(int button, int state, int x, int y)
{
	if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN))
		LIGHT_ON = true;
	else if ((button == GLUT_RIGHT_BUTTON) && (state == GLUT_DOWN))
		LIGHT_ON = false;
}

// 메인 함수
int main(int argc, char** argv)
{
	glutInit(&argc, argv);										// GLUT 초기화
	glutInitDisplayMode(GLUT_DEPTH | GLUT_RGBA | GLUT_DOUBLE);
	if (g_gamemode)
	{
		glutGameModeString("640x480:16");						// 640x480 16bpp 게임 모드 설정
		if (glutGameModeGet(GLUT_GAME_MODE_POSSIBLE))			// 게임 모드가 활성화됐다면
			glutEnterGameMode();								// 게임 모드로 변경
		else g_gamemode = false;								// 그렇지 않다면 윈도우 모드로 프로그램 실행
	}
	screen_width = glutGet(GLUT_SCREEN_WIDTH);
	screen_height = glutGet(GLUT_SCREEN_HEIGHT);
	window_width = screen_width / 1.4;
	window_height = screen_height / 1.4;

	if (!g_gamemode)
	{
		glutInitWindowSize(window_width, window_height);           // 윈도우 사이즈 설정
		glutInitWindowPosition((screen_width - window_width) / 2, (screen_height - window_height) / 2); // 프로그램 윈도우가 중앙에 위치하도록 설정
		glutCreateWindow("학연산 3층");					// 프로그램 제목
	}

	init();
	glutIgnoreKeyRepeat(true);									// 키의 반복을 무시한다.
	glutDisplayFunc(render);									// 디스플레이 이벤트 콜백 등록
	glutReshapeFunc(reshape);									// 디스플레이 재구성 이벤트 콜백 등록
	glutKeyboardFunc(keyboard);									// 키보드 이벤트 콜백 등록
	glutMouseFunc(mouse_click);									// 마우스 클릭 이벤트 콜백 등록
	glutSpecialFunc(special_keys);								// 특수키 이벤트 콜백 등록
	glutSpecialUpFunc(special_keys_up);							// 특수키가 누른 상태에서 눌러지지 않은 상태로 변화했을 때 호출되는 이벤트 콜백 등록
	glutIdleFunc(game_function);								// idle 이벤트 콜백 등록
	glutMainLoop();												// 무한 루프 실행
	return 0;
}
