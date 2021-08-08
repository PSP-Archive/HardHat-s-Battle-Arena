struct CutScene;
struct Duel;

struct CutScene *initCutScene();
void drawCutScene(struct CutScene *cs);
int updateCutScene(struct CutScene *cs,int elapsed);
int handleCutScene(struct CutScene *cs,Buttons button);
void freeCutScene(struct CutScene *cs);
void selectCutScene(struct CutScene *cs,int i,struct Duel *duel);
