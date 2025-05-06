#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBETC.H>
#include <stdio.h>

SVECTOR model_mesh[] = {
    {64, 63, -64},
    {64, -64, -64},
    {-64, -63, -64},
    {-63, 64, -64},
    {64, 63, 64},
    {63, -64, 64},
    {-64, -63, 64},
    {-63, 64, 64},
};

SVECTOR model_normals[] = {
    {-63, 64, 64},
    {-63, 64, 64},
    {-63, 64, 64},
    {-63, 64, 64},
    {-63, 64, 64},
    {-63, 64, 64},
    {-63, 64, 64},
    {-63, 64, 64},
};

int model_indices[] = {
    0, 1, 2,
    0, 2, 3,
    4, 7, 6,
    4, 6, 5,
    0, 4, 5,
    0, 5, 1,
    1, 5, 6,
    1, 6, 2,
    2, 6, 7,
    2, 7, 3,
    4, 0, 3,
    4, 3, 7,
};

CVECTOR model_colors[] = {
    138, 17, 92, 0,
    138, 17, 92, 0,
    254, 26, 105, 0,
    254, 26, 105, 0,
    0, 78, 115, 0,
    0, 78, 115, 0,
    159, 172, 58, 0,
    159, 172, 58, 0,
    77, 59, 156, 0,
    77, 59, 156, 0,
    27, 34, 182, 0,
    27, 34, 182, 0,
};

TMESH model = {
    model_mesh,
    model_normals,
    0,
    model_colors,
    6
};

#define VMODE       0
#define SCREENXRES 320
#define SCREENYRES 240
#define CENTERX     SCREENXRES/2
#define CENTERY     SCREENYRES/2
#define OTLEN       2048        // Maximum number of OT entries
#define PRIMBUFFLEN 32768       // Maximum number of POLY_GT3 primitives
// Display and draw environments, double buffered
DISPENV disp[2];
DRAWENV draw[2];
u_long      ot[2][OTLEN];                   // Ordering table (contains addresses to primitives)
char        primbuff[2][PRIMBUFFLEN]; // Primitive list // That's our prim buffer
char * nextpri = primbuff[0];                       // Primitive counter
short           db  = 0;                        // Current buffer counter
// Prototypes
void init(void);
void display(void);
//~ void LoadTexture(u_long * tim, TIM_IMAGE * tparam);
void init(){
    // Reset the GPU before doing anything and the controller
    PadInit(0);
    ResetGraph(0);
    // Initialize and setup the GTE
    InitGeom();
    SetGeomOffset(CENTERX, CENTERY);        // x, y offset
    SetGeomScreen(CENTERX);                 // Distance between eye and screen  
        // Set the display and draw environments
    SetDefDispEnv(&disp[0], 0, 0         , SCREENXRES, SCREENYRES);
    SetDefDispEnv(&disp[1], 0, SCREENYRES, SCREENXRES, SCREENYRES);
    SetDefDrawEnv(&draw[0], 0, SCREENYRES, SCREENXRES, SCREENYRES);
    SetDefDrawEnv(&draw[1], 0, 0, SCREENXRES, SCREENYRES);
    if (VMODE)
    {
        SetVideoMode(MODE_PAL);
        disp[0].screen.y += 8;
        disp[1].screen.y += 8;
    }
    SetDispMask(1);                 // Display on screen        
    setRGB0(&draw[0], 0, 128, 255);
    setRGB0(&draw[1], 0, 128, 255);
    draw[0].isbg = 1;
    draw[1].isbg = 1;
    PutDispEnv(&disp[db]);
    PutDrawEnv(&draw[db]);
    // Init font system
    FntLoad(960, 0);
    FntOpen(16, 16, 196, 64, 0, 256);
    }
void display(void){
    DrawSync(0);
    VSync(0);
    PutDispEnv(&disp[db]);
    PutDrawEnv(&draw[db]);
    DrawOTag(&ot[db][OTLEN - 1]);
    db = !db;
    nextpri = primbuff[db];
    }
int main() {
    int     i;
    int     PadStatus;
    int     TPressed=0;
    int     AutoRotate=1;
    long    t, p, OTz, Flag;                // t == vertex count, p == depth cueing interpolation value, OTz ==  value to create Z-ordered OT, Flag == see LibOver47.pdf, p.143
    POLY_G3 *poly = {0};                           // pointer to a POLY_G4 
    SVECTOR Rotate={ 232, 232, 0, 0 };                   // Rotation coordinates
    VECTOR  Trans={ 0, 0, CENTERX * 3, 0 };     // Translation coordinates
                                            // Scaling coordinates
    VECTOR  Scale={ ONE/2, ONE/2, ONE/2, 0 };     // ONE == 4096
    MATRIX  Matrix={0};                     // Matrix data for the GTE
    init();
    // Main loop
    while (1) {
        // Read pad status
        PadStatus = PadRead(0);
        if (AutoRotate == 0) {
            if (PadStatus & PADL1) Trans.vz -= 4;
            if (PadStatus & PADR1) Trans.vz += 4;
            if (PadStatus & PADL2) Rotate.vz -= 8;
            if (PadStatus & PADR2) Rotate.vz += 8;
            if (PadStatus & PADLup)     Rotate.vx -= 8;
            if (PadStatus & PADLdown)   Rotate.vx += 8;
            if (PadStatus & PADLleft)   Rotate.vy -= 8;
            if (PadStatus & PADLright)  Rotate.vy += 8;
            if (PadStatus & PADRup)     Trans.vy -= 2;
            if (PadStatus & PADRdown)   Trans.vy += 2;
            if (PadStatus & PADRleft)   Trans.vx -= 2;
            if (PadStatus & PADRright)  Trans.vx += 2;
            if (PadStatus & PADselect) {
                Rotate.vx = Rotate.vy = Rotate.vz = 0;
                Scale.vx = Scale.vy = Scale.vz = ONE/2;
                Trans.vx = Trans.vy = 0;
                Trans.vz = CENTERX * 3;
            }
        }
        if (PadStatus & PADstart) {
            if (TPressed == 0) {
                AutoRotate = (AutoRotate + 1) & 1;
                Rotate.vx = Rotate.vy = Rotate.vz = 0;
                Scale.vx = Scale.vy = Scale.vz = ONE/2;
                Trans.vx = Trans.vy = 0;
                Trans.vz = CENTERX * 3;
            }
            TPressed = 1;
        } else {
            TPressed = 0;
        }
        if (AutoRotate) {
            Rotate.vy += 28; // Pan
            Rotate.vx += 28; // Tilt
            //~ Rotate.vz += 8; // Roll
        }
        // Clear the current OT
        ClearOTagR(ot[db], OTLEN);
        // Convert and set the matrixes
        RotMatrix(&Rotate, &Matrix);
        TransMatrix(&Matrix, &Trans);
        ScaleMatrix(&Matrix, &Scale);
        SetRotMatrix(&Matrix);
        SetTransMatrix(&Matrix);
        // Render the sample vector model
        t=0;
        // modelCube is a TMESH, len member == # vertices, but here it's # of triangle... So, for each tri * 3 vertices ...
        for (i = 0; i < (model.len*3); i += 3) {               
            poly = (POLY_G3 *)nextpri;
            // Initialize the primitive and set its color values
            SetPolyG3(poly);
            setRGB0(poly, model.c[i].r , model.c[i].g   , model.c[i].b);
            setRGB1(poly, model.c[i+2].r, model.c[i+2].g, model.c[i+2].b);
            setRGB2(poly, model.c[i+1].r, model.c[i+1].g, model.c[i+1].b);
            // Rotate, translate, and project the vectors and output the results into a primitive
            OTz  = RotTransPers(&model_mesh[model_indices[t]]  , (long*)&poly->x0, &p, &Flag);
            OTz += RotTransPers(&model_mesh[model_indices[t+2]], (long*)&poly->x1, &p, &Flag);
            OTz += RotTransPers(&model_mesh[model_indices[t+1]], (long*)&poly->x2, &p, &Flag);
            // Sort the primitive into the OT
            OTz /= 3;
            if ((OTz > 0) && (OTz < OTLEN))
                AddPrim(&ot[db][OTz-2], poly);
            nextpri += sizeof(POLY_G3);
            t+=3;
        }
        FntPrint("cubick!\n");
        FntFlush(-1);
        display();
    }
    return 0;
}