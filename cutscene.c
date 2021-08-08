#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pspkernel.h>
#include <pspgu.h>
#include <pspgum.h>
#include "main.h"
#include "duel.h"

struct CSActor {
	char name[128];
	struct AnimMesh *mesh;
	Image *skin;
	Image *icon;

	// for MD2
	int frameElapsed;
	int lastFrame;
	int frame;
	enum MotionType motion;

	float x,y,z,angle;	// target position
};

typedef enum SpeakerType { ST_HERO, ST_KING, ST_BROTHER } SpeakerType;

struct CSDialog {
	char *message;
	SpeakerType speaker;	// the actor speaking
	float dist1;
	float dist2;
};

struct CSDialog csDialog0[]={
{"Vidar! How dare you show yourself in Yggdrasil!",ST_KING,100,60},
{"Relax Heimdallr, I come peacfully.",ST_BROTHER,40,40},
{"You will address him as King or father you traitor!",ST_HERO,40,40},
{"Be quiet, Njord! Let me deal with this.",ST_KING,100,100},
{"Yes, leave this matter to your aging dragon of a father.",ST_BROTHER,40,40},
{"Why you-!",ST_HERO,60,60},
//HEIMDALLR: Enough! How dare you use your powers here, Vidar!
{"The little twerp should not start things he can't finish.",ST_BROTHER,30,30},
{"Why are you here, Vidar? Enough of this game.",ST_KING,100,100},
{"I will cut to the chase then. I am here to tell you that Yggdrasil has one month before the kingdom of Hall declares war on it.",ST_BROTHER,100,100},
{"What is this nonsense? We have done nothing to offend your kingdom.",ST_KING,30,30},
{"Yes, but our king does seem to like this land very much and if you do not give it to him we will declare war on you.",ST_BROTHER,30,30},
{"We know you have no champion so I suggest you simply hand over control and be done with it.",ST_BROTHER,30,30},
{ "I will not deal with those who would betray their own family and especially not with Hall!",ST_KING,60,60},
{ "Nevertheless, you have been warned. Hand over Yggdrasil or we will find out whether or not you can still handle yourself in battle.",ST_BROTHER,60,60},
{"As you know, if a kingdom has no champion it is their king's duty to act as the champion and fight.",ST_BROTHER,60,120},
	//VIDAR turns and leaves the thrown. NJORD moves to his father's side.
{"Father, you surely do not believe you could ever defeat Vidar in battle, do you?",ST_HERO,40,30},
{"Not I, Njord, but you can. I have arranged to speed up your training - you will be going to Sushi Island.",ST_KING,30,30},
{"Are you saying that I am the new champion?",ST_HERO,30,30},
{"Yes, son. You will be leaving today. We only have a few weeks to get ready.",ST_KING,30,30},
{"If you face Vidar as our champion and win, we can divert war and keep our land. It is the only way.",ST_KING,30,30},
{"I... I understand father.",ST_HERO,30,30},
};
struct CSDialog csDialog1[]={
{"It seems your training is progressing well, son.",ST_KING,30,30},
{"Yes, father. I am already training my first mage.",ST_HERO,30,30},
//	Enter VIDAR.
{"Well, well, so it seems you have survived your first days of Sushi Island.",ST_BROTHER,100,100},
{"Why are you here? What do you want?",ST_HERO,80,80},
{"Oh, I'm just here to see how things are going, Njord.",ST_BROTHER,40,40},
{"I find that hard to believe, Vidar.",ST_HERO,60,60},
{"Are you suggesting that I am here for some sort of nefarious reason? Why I can't believe you would think such a thing of me.",ST_BROTHER,30,30},
{"Just get on with it, Vidar. What do you want?",ST_KING,80,80},
{"I am merely here to check on Njord's progress in the Sushi Island tournament. What do you say, Njord?",ST_BROTHER,60,60},
{"Shall we do battle and see how ready you are?",ST_BROTHER,60,60},
{"We'll do battle alright. And I'll see to it I whip you thoroughly.",ST_HERO,30,100},
{"I would hope you can back up those words.",ST_BROTHER,30,100},
};
struct CSDialog csDialog2[]={
{"Well, it seems you have indeed made some progress in your training, little brother.",ST_BROTHER,100,50},
{"But you still have much to learn if you are to be a full-fledged mage master. I doubt you will achieve that level, though.",ST_BROTHER,50,50},
{"Are you saying that only because you know I will beat you when the time comes to battle?",ST_HERO,60,60},
{"Hmph! If you think I am afraid of you, you are quite wrong. I will utterly destroy you and father when it is time. ",ST_BROTHER,40,40},
{"You don't have to say it. I can see that you are fearful.",ST_HERO,50,50},
{"As I said, when the time comes you will be destroyed. Do no delude yourself, swine.",ST_BROTHER,60,60},
{"Well, we will see just who is the better of us once and for all soon enough, Vidar. I will defeat you.",ST_HERO,40,40},
{"Haha, only an imbecile would believe that they could best me.",ST_BROTHER,60,60},
{"Again, we will see.",ST_HERO,80,80},
{"Yes, we shall, Njord. And when it is time I will enjoy having you and father working as my personal servants.",ST_BROTHER,40,40},
{"You will serve my every need and I will make sure you do not ignore. And this land will be beautiful to become the king of. Oh, how fun that shall be!",ST_BROTHER,40,100},
};struct CSDialog csDialog3[]={
//	We see NJORD and HEIMDALLR having a conversation in front of the throne.
{"I have consulted the scrying bowl, son...",ST_KING,120,60},
{"What is it father? What did you see in the scrying bowl?",ST_HERO,80,80},
{"You could take a look yourself...",ST_KING,120,120},
// HEIMDALLR points to the scrying bowl.
{"But, I suppose it is best that I tell you.",ST_KING,60,60},
{"What, father? What did you see?",ST_HERO,30,30},
{"The future I have seen shows our kingdom falling to the hands of Vidar's - the kingdom Hall. I do not think we will make it.",ST_KING,50,50},
{"We have been training you for a lost cause.",ST_KING,50,50},
{"But, father, the scrying bowl is not always right. And even if it is, it does not mean we cannot change the future.",ST_HERO,80,80},
{"Yes, Njord, I suppose you are correct but...",ST_KING,50,50},
{"But nothing, father. We will get out of this fine. I must go now as Vidar has arrived for another tournament but, rest assured.",ST_HERO,40,40},
{"We will be alright at the end of this. I guarantee it, father.",ST_HERO,40,100},
};
struct CSDialog csDialog4[]={
//	NJORD and VIDAR are on Sushi Island.
{"It seems I have passed another of your 'tests' again, Vidar.",ST_HERO,100,100},
{"Nevertheless, on the true battlefield you will fall easily.",ST_BROTHER,60,60},
{"The point is that I am making progress. Even you and your hard-headed mind cannot deny that.",ST_HERO,40,40},
{"Ha! If you think you are making any progress, you are sadly mistaken. I've seen first year Mage Masters who have made more progress than you.",ST_BROTHER,80,80},
{"You are simply a pathetic little boy playing at a man's game. I could destroy you in an instant if I wanted.",ST_BROTHER,80,80},
{"But you won't. Your leash ties you to the orders of your master in Hall. I doubt you could do what you wanted.",ST_HERO,90,90},
{"Why you-! I am a full-fledged mage master who answers to no one. Not even the king of Hall!",ST_BROTHER,50,50},
{"Then why are you continually running around doing errands for your masters?",ST_HERO,70,70},
{"I don't have to listen to an immature brat. I will be back but I doubt you'll have made any progress by then.",ST_BROTHER,30,100},
};
struct CSDialog csDialog5[]={
//	We see a close-up of NJORD. VIDAR is also present in the scene.
{"You have returned to test me again, I see.",ST_HERO,30,30},
{"And why would I not? I care much to see how far my younger brother has progressed.",ST_BROTHER,60,60},
{"I see you have gotten over your anger from the last time, Vidar.",ST_HERO,50,50},
{"My anger? You seem to have misread me. I was merely excited not angry so of course I would not be angry today. In fact, I feel absolutely happy.",ST_BROTHER,70,70},
{"I am glad to see that. But shall we not get back to the matter at hand?",ST_HERO,50,50},
{"Oh, of course! Your little training - your progress. Let us see how much better you have gotten since the last time. Shall we begin?",ST_BROTHER,100,100},
//	NJORD seems slightly confused over VIDAR's attitude.
{"Um, yes. Let's.",ST_HERO,120,120},
};
struct CSDialog csDialog6[]={
//	We are on Sushi Island. On the field is only NJORD and his father, HEIMDALLR.
{"I watched the tournament, Njord. You did well. You are becoming a more powerful mage master than I could ever have imagined.",ST_KING,70,70},
{"Thank you, father...",ST_HERO,60,60},
{"What is the matter, Njord? Speak up.",ST_KING,80,80},
{"It's just- did you notice Vidar, by any chance? He was acting... strange. As if he was too happy.",ST_HERO,30,30},
{"Oh, Vidar... I am not sure but... well I have heard rumors... I'm not sure.",ST_KING,80,80},
{"Rumors? What rumors father?",ST_HERO,40,40},
{"They say that Vidar has assassinated the king of Hall.",ST_KING,100,100},
{"He murdered the - wait, what?!",ST_HERO,30,30},
{"It seems he is no longer just the champion of Hall but also its... king. I may be wrong though. This is just a rumor...",ST_KING,70,70},
{"I- I- this is crazy. Vidar would never murder anyone! I know him. He may have changed his allegiance but I know my brother.",ST_HERO,40,40},
{"Vidar is not the same person you once knew, Njord. He has changed. This is even more reason why you must complete your training swiftly.",ST_KING,50,50},
{"Yes, father. I know...",ST_HERO,40,100},
};
struct CSDialog csDialog7[]={
//	On Sushi Island we see VIDAR and NJORD talking. They are standing a bit away from each other.
{"So, it true, Vidar?",ST_HERO,80,80},
{"Is what true, my delightful little brother?",ST_BROTHER,40,40},
{"Have you murdered the king of Hall? Tell me, is it true?",ST_HERO,30,30},
{"Oh, murder? Me murder someone?",ST_BROTHER,60,60},
//We see VIDAR from NJORD's shoulder.
{"Answer the question, Vidar.",ST_HERO,70,70},
{"Well, of course I did not murder him. That's the wrong word. I prefer \"disposed\" of him.",ST_BROTHER,40,40},
{"...How could you do such a thing?",ST_HERO,30,30},
{"Oh, I can't take credit for that, Njord. It was all you. I took your advice and cut the leash that was holding me back.",ST_BROTHER,70,70},
{"But, I never-",ST_HERO,80,80},
{"Don't be modest, Njord! You must take credit for your work. It was a job well done. Oh, do I feel so free.",ST_BROTHER,50,50},
{"Eh- you- this-!",ST_HERO,90,90},
{"Stop babbling, little brother. Why don't we get back to what we were supposed to be doing - our little mage master games.",ST_BROTHER,30,100},
};
struct CSDialog csDialog8[]={
//	It is right after the tournament and we see NJORD speaking with HEIMDALLR in the throne.
{"He has become - I don't know... he's not Vidar anymore. I don't know who he is, father.",ST_HERO,100,100},
{"I told you, son, he is not the Vidar we once knew.",ST_KING,60,60},
{"Not the same Vidar that used to play games with you as a child. He is not that Vidar. Not since-",ST_KING,60,60},
{"Let's not speak of it...",ST_HERO,40,40},
{"Yes, it is best to leave the past in the past and deal with the present.",ST_KING,70,70},
{"Father, the scrying bowl... how sure are you of what you saw. Was it an unclear vision? Something that is uncertain?",ST_HERO,60,60},
{"No, Njord. The scrying bowl showed me this future as clear as you see me now. But, as you said, we may yet still be able to change it.",ST_KING,100,100},
{"I don't know, father. If Vidar is truly this ruthless and changed then it-",ST_HERO,60,60},
{"It changes nothing, Njord. I was a fool for even believing for a second that our future will be bleak.",ST_KING,70,70},
{"We will make it through this. You just need to believe that, son. Now, you should get back to Sushi Island. I think it is best you continue your training. We have little time left.",ST_KING,70,70},
{"Yes, father, I'll see you tomorrow. I will do my best to progress quickly. ",ST_HERO,80,80},
{"Just remember that the people of Yggdrasil are counting on you.",ST_KING,90,90},
{"I will.",ST_HERO,60,100},
};
struct CSDialog csDialog9[]={
//	We see HEIMDALLR standing next to NJORD with VIDAR parallel to the two on Sushi Island.
{"So you have come to watch the two of us play, have you, Heimdallr? Enjoy watching the two of us play mage master?",ST_BROTHER,120,120},
{"I have merely come to check on Njord's progress.",ST_KING,80,80},
{"Oh, ho, ho. Is that all?",ST_BROTHER,50,50},
{"Yes, Vidar... Why are you acting so strange?",ST_KING,60,60},
{"Strange? Why I'm just happy that you are here to watch us. We'll do our best to put on quite a show - right, Njord?",ST_BROTHER,90,90},
{"Let's just get on with it. Enough wasting time, Vidar.",ST_HERO,60,60},
{"What's the hurry? You can't wait to finish your training so that you can lose the battle for the kingdom and become my servant?",ST_BROTHER,40,40},
{"That's not going to happen, Vidar.",ST_HERO,80,80},
{"Oh, but it will. I heard about father's little foray with the scrying bowl.",ST_BROTHER,40,40},
{"It seems that things are going to go well for me in the end.",ST_BROTHER,40,40},
{"Stop wasting time. Let's go.",ST_BROTHER,100,30},
};
struct CSDialog csDialog10[]={
//	We see HEIMDALLR, NJORD, and VIDAR on the field of Sushi Island again.
{"Better, better, but still not quite enough. You have very bad control over you mages still.",ST_BROTHER,100,100},
{"What are you talking about? Njord handled himself quite well with his mages. He is much better than you ever were at this stage.",ST_KING,50,50},
{"Oh, be quiet you old man. Your memory fails you. I, in fact, was a much better mage master in this stage of training than Njord is.",ST_BROTHER,30,30},
{"I passed this tournament, that is what matters.",ST_HERO,70,70},
{"Sure, to a beginner. But to a real mage master this would be child's play. You were struggling all the while to win.",ST_BROTHER,60,60},
{"I fear that the scrying bowl was right and you really shall be on the losing end of this whole thing.",ST_BROTHER,60,60},
{"You are no longer under the command of the king of Hall - you are its king. You can call of this whole thing, Vidar.",ST_KING,50,50},
{"And miss out on all the fun I'll have ruling Yggdrasil? I think not! I'm going to enjoy this so much, you just have no idea!",ST_BROTHER,40,40},
{"It will really be fun seeing you and Njord suffer at my hand. You'll get back everything you deserve.",ST_BROTHER,40,40},
{"I know that you think that your mother's dea-",ST_KING,30,30},
{"Be quiet! Don't you dare speak of her again. If you so much as think about talking about this again I will kill you, old man.",ST_BROTHER,40,40},
{"Vidar-",ST_KING,60,60},
{"I must be going now. I shall see you again soon. Be ready for the last tournament, Njord.",ST_BROTHER,50,50},
{"Hopefully you'll be able to call yourself a full-fledged mage master by the time we do battle. Ta-ta for now.",ST_BROTHER,50,120},
};
struct CSDialog csDialog11[]={
//	We are in the throne with HEIMDALLR and NJORD.
{"Today is the final tournament, father.",ST_HERO,100,100},
{"Yes, if you win today you can be considered a fully-fledged mage master.",ST_KING,60,60},
{"I hope you can win for Yggdrasil's sake. If you do not pass then it is a sign that you are not ready for the battle that will decide our kingdom's fate...",ST_KING,60,60},
{"Do not worry, father, I know I can win. When I return I will be a full-fledged mage master - I will be ready to take on Vidar in a true battle.",ST_HERO,70,70},
{"I hope so, son. I truly do. If Vidar were to win, his reign over Yggdrasil would be-",ST_KING,30,30},
{"He will not. I will make sure of it. You can rest peacefully knowing that.",ST_HERO,40,40},
{"Then I shall take your word for it, Njord.",ST_KING,60,60},
{"Despite what the scrying bowl has shown us I do believe that with you as Yggdrasil's champion, we can get through this.",ST_KING,60,60},
{"Yes, we will be fine. I do not think the gods would allow Vidar to win control of Yggdrasil.",ST_HERO,70,70},
{"That is my hope, Njord. Now, may the gods be with you. Come back bearing the title of \"Mage Master.\"",ST_KING,90,90},
{"I will, father.",ST_HERO,50,100},
};
struct CSDialog csDialog12[]={
};
struct CSDialog csDialog13[]={
};
struct CSDialog csDialog14[]={
//	In the throne we see HEIMDALLR, NJORD, and VIDAR.
{"NOOOOOOOOo! I- I- I could not have lost! Not to you! Not to an insolent brat! No!",ST_BROTHER,100,100},
{"Accept your defeat, Vidar. I beat you. Yggdrasil stays in its rightful hands. ",ST_HERO,60,60},
{"This is impossible! No!",ST_BROTHER,80,80},
{"But it is possible and it has happened, Vidar. No matter how much you think I could not beat you, it happened.",ST_HERO,80,80},
{"You should leave Yggdrasil unless you would like me to deliver you a blow so powerful you will not be able to walk out of this kingdom of your own free will.",ST_HERO,80,80},
{"This is not over you brat. I will not accpt this.",ST_BROTHER,30,30},
{"Whatever you like, Vidar. Just leave.",ST_HERO,100,100},
//	Vidar leaves. We have a close-up of Heimdallr.
{"You did it, Njord! You have saved Yggdrasil from the future the scrying bowl saw.",ST_KING,30,30},
{"Yes, father.",ST_HERO,40,40},
{"Why are you so sad? We should rejoice, son.",ST_KING,35,35},
{"I am sad because I have finally realized that I have truly lost my brother. And I know this will not be the end of it.",ST_HERO,30,30},
{"He will be back. And maybe next time it will be a battle to the death. And maybe next time, I won't be the victor...",ST_HERO,30,30},
{"Njord, I understand how you feel. But for the moment we must forget the negative things.",ST_KING,40,40},
{"They are but the way of our life. We cannot let them decide our outlook. Vidar may be vengeful now but who is to say that he will not change?",ST_KING,40,40},
{"Who is to say that he will not become enlightened? That the gods may change his ways. That he may return to us as the Vidar we once knew?",ST_KING,40,40},
{"Maybe you are right, father. Maybe I'll soon have the older brother that I have so long forgotten about.",ST_HERO,30,30},
{"For now you have proven yourself as a worthy champion of Yggdrasil.",ST_KING,40,40},
{"You have proven that you can lead Yggdrasil to victory if ever the time arises again that our honor, our people, our land need be defended.",ST_KING,40,40},
{"I hope I truly can live up to that statement, father.",ST_HERO,35,35},
{"I know you can live up to it, Njord. You were always meant for great things. There is no evil that will not meet their reckoning should they face you.",ST_KING,40,40},
{"I really hope that the time will not come soon when I have to do battle with Vidar again, though. ",ST_HERO,30,30},
{"Rest assured, son. Vidar will not come back any time soon.",ST_KING,40,40},
{"And if he does come back, not my son, not as your brother, but as an enemy of Yggdrasil, you will be ready.",ST_KING,40,40},
};

enum CastType { CT_TRIO, CT_BROS, CT_FATHERSON };
enum LocationType { LT_IN, LT_OUT };	// inside the throne room, or on tournament field.
struct CSScene {
	enum CastType cast;
	enum LocationType location;
	struct CSDialog *dialog;
	int dialogCount;
} csScene[]={
	{CT_TRIO,LT_IN,csDialog0, sizeof(csDialog0)/sizeof(struct CSDialog)},
	// tournament 1
	{CT_TRIO,LT_IN,csDialog1, sizeof(csDialog1)/sizeof(struct CSDialog)},
	{CT_BROS,LT_OUT,csDialog2, sizeof(csDialog2)/sizeof(struct CSDialog)},
	// tournament 2
	{CT_FATHERSON,LT_IN,csDialog3, sizeof(csDialog3)/sizeof(struct CSDialog)},
	{CT_BROS,LT_OUT,csDialog4, sizeof(csDialog4)/sizeof(struct CSDialog)},
	// tournament 3
	{CT_BROS,LT_OUT,csDialog5, sizeof(csDialog5)/sizeof(struct CSDialog)},
	{CT_FATHERSON,LT_OUT,csDialog6, sizeof(csDialog6)/sizeof(struct CSDialog)},
	// tournament 4
	{CT_BROS,LT_OUT,csDialog7, sizeof(csDialog7)/sizeof(struct CSDialog)},
	{CT_FATHERSON,LT_IN,csDialog8, sizeof(csDialog8)/sizeof(struct CSDialog)},
	// tournament 5
	{CT_TRIO,LT_OUT,csDialog9, sizeof(csDialog9)/sizeof(struct CSDialog)},
	{CT_TRIO,LT_OUT,csDialog10, sizeof(csDialog10)/sizeof(struct CSDialog)},
	// tournament 6
	{CT_FATHERSON,LT_IN,csDialog11, sizeof(csDialog11)/sizeof(struct CSDialog)},
//	{CT_FATHERSON,LT_OUT,csDialog12, sizeof(csDialog12)/sizeof(struct CSDialog)},
	// final battle
//	{CT_TRIO,LT_IN,csDialog13, sizeof(csDialog13)/sizeof(struct CSDialog)},
	{CT_TRIO,LT_IN,csDialog14, sizeof(csDialog14)/sizeof(struct CSDialog)},
};
extern struct AnimMesh *brickMesh;

//struct CSDialog *csDialog=csDialog0;
//int csDialogCount=17;

struct CutScene {
	struct AnimMesh *brickMesh;
	struct CSActor actor[3];
	int dialogPos;
	int dialogElapsed;
	int throneId;
	struct Duel *duel;
	struct CSScene *scene;
};

void loadCSActor(struct CSActor *actor,char *name,struct AnimMesh *mesh)
{
	char buf[256];
	strcpy(actor->name,name);
	sprintf(buf,"models/brick/%s_ii.png",name);
	actor->icon=loadPng(buf);
	swizzleFast(actor->icon);
	sprintf(buf,"models/brick/%s.png",name);
	actor->skin=loadPng(buf);
	swizzleFast(actor->skin);
	actor->motion=MT_STAND;
	actor->mesh=mesh;
}

void setCSActorPos(struct CSActor *actor,float x,float z,float angle)
{
	actor->x=x;
	actor->y=0;
	actor->z=z;
	actor->angle=angle;
}

void freeCSActor(struct CSActor *actor)
{
	freeImage(actor->icon);
	freeImage(actor->skin);	
}

struct CutScene *initCutScene()
{
	struct CutScene *cs=calloc(sizeof(struct CutScene),1);

	//freeItems();	// free all of the models.
	cs->throneId=loadItemImage("buildings/throneroom");
	if(brickMesh) {
		cs->brickMesh=brickMesh;
	} else {
		cs->brickMesh=md2Load("models/brick/tris.md2");
		brickMesh=cs->brickMesh;
	}
	loadCSActor(&cs->actor[ST_HERO],"blue",cs->brickMesh);
	loadCSActor(&cs->actor[ST_BROTHER],"yellow",cs->brickMesh);
	loadCSActor(&cs->actor[ST_KING],"ice2001s",cs->brickMesh);
	setCSActorPos(cs->actor+ST_HERO,25,60,(GU_PI*2)*1/3);
	setCSActorPos(cs->actor+ST_KING,-25,60,(GU_PI*2)*2/3);
	setCSActorPos(cs->actor+ST_BROTHER,0,100,(GU_PI*2)*0/3);
	cs->scene=csScene+0;

	return cs;
}

void selectCutScene(struct CutScene *cs,int i,struct Duel *duel)
{
	cs->scene=csScene+i;

	if(cs->scene->cast==CT_TRIO) {	
		setCSActorPos(cs->actor+ST_HERO,25,60,(GU_PI*2)*1/3);
		setCSActorPos(cs->actor+ST_KING,-25,60,(GU_PI*2)*2/3);
		setCSActorPos(cs->actor+ST_BROTHER,0,100,(GU_PI*2)*0/3);
	} else if(cs->scene->cast==CT_BROS) {
		setCSActorPos(cs->actor+ST_HERO,25,60,(GU_PI*2)*1/6);
		setCSActorPos(cs->actor+ST_KING,-5000,0,0);
		setCSActorPos(cs->actor+ST_BROTHER,-25,60,(GU_PI*2)*4/6);
	} else if(cs->scene->cast==CT_FATHERSON) {
		setCSActorPos(cs->actor+ST_HERO,25,60,(GU_PI*2)*1/6);
		setCSActorPos(cs->actor+ST_KING,-25,60,(GU_PI*2)*4/6);
		setCSActorPos(cs->actor+ST_BROTHER,-5000,0,0);
	}
	if(cs->scene->location==LT_IN) {
		//freeItems();
		//cs->throneId=loadItemImage("buildings/throneroom");
	} else {
		//freeItems();
		//loadItems();
	}
	
	cs->dialogElapsed=0;
	cs->dialogPos=0;
	cs->duel=duel;
	loadCharacters(0,0);	// hide the characters, so that we can supply our own.
}

void drawCSActor(struct CSActor *actor)
{
	if(!actor->mesh) return;
	int frameDuration=1000/animMotion[actor->motion].fps;
	float fraction=(actor->frameElapsed%frameDuration)/(float)frameDuration;
	int frame,nextFrame;
	frame=actor->lastFrame;
	nextFrame=actor->frame;

	struct AnimMesh *mesh=actor->mesh;
	struct Vertex3DTP *buffer=(struct Vertex3DTP *)sceGuGetMemory( sizeof(struct Vertex3DTP)*mesh->polyCount*3);
	//if(model->cullBackface==0) buffer=(struct Vertex3DTP *)(0x0fffffff&(int)buffer);
	buffer=(struct Vertex3DTP *)((int)buffer&~0x40000000);
	float minMax[6];
	md2GetFractionalFrame(mesh,frame,nextFrame,fraction,buffer,minMax);
	sceKernelDcacheWritebackAll();

	Image *source=actor->skin;
	if(source) {
		sceGuTexMode(GU_PSM_8888, 0, 0, source->isSwizzled);
		sceGuTexImage(0, source->textureWidth, source->textureHeight, source->textureWidth, source->data);
	}
	sceGumMatrixMode(GU_MODEL);
	sceGumPushMatrix();
	/*
	int i;
	for(i=0;i<6;i++) printf("minMax[%d]=%.2f; ",i,minMax[i]);
	printf("\n");
	*/
	ScePspFVector3 trans={actor->x,actor->y+25,actor->z};
	sceGumTranslate(&trans);
	sceGumRotateY(actor->angle);
	sceGumDrawArray(GU_TRIANGLES,GU_VERTEX_32BITF|GU_TEXTURE_32BITF|GU_TRANSFORM_3D,mesh->polyCount*3,0,buffer);
	sceGumPopMatrix();
	//printf("acting\n");
}

void drawCutScene(struct CutScene *cs)
{
	sceKernelDcacheWritebackAll();	// superstitious, I guess, but not much time left.
	sceGumPushMatrix();
	if(cs->scene->location==LT_IN) {
		ScePspFVector3 scale={5,5,5};
		sceGumScale(&scale);
		renderItem(cs->throneId);
	} else {
		drawDuelSuspended(cs->duel);
	}
	sceGumPopMatrix();
	sceGuFrontFace(GU_CW);
	drawCSActor(cs->actor+0);
	drawCSActor(cs->actor+1);
	drawCSActor(cs->actor+2);
	// Now draw the current dialog.
	if(cs->dialogPos<cs->scene->dialogCount) {
		SpeakerType who=cs->scene->dialog[cs->dialogPos].speaker;
		who=(SpeakerType)(who%3);
		drawFilledRect(0,202,480,272-202,GU_RGBA(0,0,0,0x80));
		drawSprite(0,0,64,64,cs->actor[who].icon,2,202-32);
		intraFontPrintColumn(iFont[FONT_MESSAGE],68,202+18,480-68,cs->scene->dialog[cs->dialogPos].message);
	} else {
		intraFontPrint(iFont[FONT_MESSAGE],360,250,"Loading...");
	}
}

void updateCSActor(struct CSActor *actor,int elapsed)
{
//	if(actor->oldMotion!=actor->motion) {
		//printf("character %s now doing motion %s.\n",actor->name,animMotionName[actor->motion]);
//		actor->oldMotion=actor->motion;
//	}

	actor->frameElapsed+=elapsed;
	
	// animate the MD2
	struct AnimMotion *motion=animMotion+actor->motion;
	int frameMod=actor->frameElapsed/(1000/motion->fps);
	frameMod+=motion->startFrame;
	if(frameMod>motion->endFrame) {
		frameMod=motion->startFrame;
		actor->frameElapsed=0;
		// we always cycle whatever motion.
	}
	if(frameMod!=actor->frame) {
		actor->lastFrame=actor->frame;
		actor->frame=frameMod;
	}
}

int updateCutScene(struct CutScene *cs,int elapsed)
{
	float fx=50,fy=80,fz=200,tx=0,ty=40,tz=0;
	
	cs->dialogElapsed+=elapsed;

	if(cs->dialogPos<cs->scene->dialogCount) {
		SpeakerType who=cs->scene->dialog[cs->dialogPos].speaker;
		who=(SpeakerType)(who%3);
		tx=cs->actor[who].x;
		ty=cs->actor[who].y+20;
		tz=cs->actor[who].z;
		
		float cent=cs->dialogElapsed/1000.0f;
		if(cent>1) cent=1;
		float per=1-cent;
		//if(per!=0.0f) printf("Percent at %.3f\n",per);
		float dist=(cs->scene->dialog[cs->dialogPos].dist1*per+cs->scene->dialog[cs->dialogPos].dist2*cent)*1.5f;
		fx=tx+sinf(cs->actor[who].angle+GU_PI)*dist;
		fz=tz+cosf(cs->actor[who].angle+GU_PI)*dist;
		fy=ty+dist/2;
		updateCSActor(cs->actor+who,elapsed);
	}
	cameraSetFromTo(fx,fy,fz,tx,ty,tz);
	//updateCSActor(cs->actor+0,elapsed);
	//updateCSActor(cs->actor+1,elapsed);
	//updateCSActor(cs->actor+2,elapsed);
	
	return cs->dialogPos>=cs->scene->dialogCount;	// returns 1 when complete.
}

int handleCutScene(struct CutScene *cs,Buttons button)
{
	switch(button) {
	case BT_CROSS:
		cs->dialogPos++;
		cs->dialogElapsed=0;
		break;
	case BT_CIRCLE:
		cs->dialogPos=cs->scene->dialogCount;
	default:
		break;
	}
	return 0;
}

void freeCutScene(struct CutScene *cs)
{
	if(cs->brickMesh) cs->brickMesh->texture=0;
	if(brickMesh) md2Free(brickMesh);
	brickMesh=0;
	freeCSActor(cs->actor+0);
	freeCSActor(cs->actor+1);
	freeCSActor(cs->actor+2);
	freeItems();
}

#if 0
struct Trainer {
	struct CutScene *cs;
	SleakerType speaker;
	struct CSActor actor;
};

struct Trainer *initTrainer(struct CutScene *cs)
{
	struct Trainer *trainer=calloc(sizeof(struct Trainer),1);
	trainer->cs=cs;
	return trainer;
}

void drawTrainer(struct Trainer *trainer)
{
	drawCSActor(trainer->actor);
	// now draw the menu maybe?
	// Now draw the current dialog.
	if(cs->dialogPos<cs->scene->dialogCount) {
		SpeakerType who=cs->scene->dialog[cs->dialogPos].speaker;
		who=(SpeakerType)(who%3);
		drawFilledRect(0,202,480,272-202,GU_RGBA(0,0,0,0x80));
		drawSprite(0,0,64,64,cs->actor[who].icon,2,202-32);
		intraFontPrintColumn(iFont[FONT_MESSAGE],68,202+18,480-68,cs->scene->dialog[cs->dialogPos].message);
	} else {
		intraFontPrint(iFont[FONT_MESSAGE],360,250,"Loading...");
	}
}

int updateTrainer(struct Trainer *trainer,int elapsed)
{
	float fx=50,fy=80,fz=200,tx=0,ty=40,tz=0;
	
	SpeakerType who=csDialog[cs->dialogPos].speaker;
	tx=trainer->actor.x;
	ty=trainer->actor.y+20;
	tz=trainer->actor.z;
	
	//if(per!=0.0f) printf("Percent at %.3f\n",per);
	float dist=120;
	fx=tx+sinf(trainer->actor.angle+GU_PI)*dist;
	fz=tz+cosf(trainer->actor.angle+GU_PI)*dist;
	fy=ty+dist/2;
	updateCSActor(&trainer->actor,elapsed);
	cameraSetFromTo(fx,fy,fz,tx,ty,tz);
	
	return 0;	// returns 1 when complete.
}

int handleTrainer(struct Trainer *trainer,Buttons button)
{
	switch(button) {
	case BT_CROSS:
		break;
	case BT_SQUARE:
		break;
	case BT_CIRCLE:
		break;
	default:
		break;
	}
	return 0;
}
#endif
