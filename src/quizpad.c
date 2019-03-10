/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include <wchar.h>
#include "quizpad.h"
#include "tomoe-canvas.h"

#define AUTO_FIND_TIME 2000

typedef struct _QuizPadPriv QuizPadPriv;

struct _QuizPadPriv
{
    //GtkWidget       *clues;//meaning and pronunciation textview
    GtkWidget       *meaning;
    GtkWidget       *pronun;
    GtkWidget       *meaning2;
    GtkWidget       *pronun2;
    GtkWidget       *answertv; //textview for training mode
    GtkWidget       *checkm;
    GtkWidget       *checkp;
    GtkWidget       *checkm2;
    GtkWidget       *checkp2;
    GtkWidget       *checka; //checkbox for training mode
    GtkWidget       *next; //next word for trainign  mode
	GtkWidget       *word;//field to complete entry
	GtkWidget       *del_char; //delete a character button
	GtkWidget       *correct; //send the word contained in word to correct it button
    GtkWidget       *answer; //get the answer
    GtkWidget       *pronounce;
    GtkWidget       *pronouncealt;
	GtkWidget       *pad1; //TomoeCanvas
	GtkWidget       *pad2; //TomoeCanvas

    GtkWidget       *nextg_button; //next button for training mode
    GtkWidget       *nextg_label; //next label for training mode

    GtkWidget       *qcontainer; //quiz container vbox
    GtkWidget       *trcontainer; //trcontainer

    GString         *smeaning;
    GString         *spronun;
    GString         *sanswer;
    gboolean        mode; //FALSE=Quiz, TRUE= training 
};


#define QUIZPAD_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TYPE_QUIZPAD, QuizPadPriv))

G_DEFINE_TYPE (QuizPad, quiz_pad, GTK_TYPE_FRAME)

static guint signal_value[6];

/* virtual functions for GtkObject class */
static void   quiz_pad_dispose          (GObject           *object);
void quiz_pad_add_kanji_from_canvas          (TomoeCanvas *canvas, gpointer data);

static void
quiz_pad_class_init(QuizPadClass* klass)
{

    GObjectClass   *gobject_class = G_OBJECT_CLASS (klass);
    //GtkFrameClass  *frame_class  = GTK_WIDGET_CLASS (klass);
    //GParamSpec *spec;

    gobject_class->dispose             = quiz_pad_dispose;
    
    signal_value[0]=g_signal_new ("correct",
                        G_TYPE_FROM_CLASS (klass),
                        G_SIGNAL_RUN_LAST,
                        0,
  		                NULL, NULL,
  		                g_cclosure_marshal_VOID__VOID,
                 		G_TYPE_NONE, 0);

    signal_value[1]=g_signal_new ("answer",
                        G_TYPE_FROM_CLASS (klass),
                        G_SIGNAL_RUN_LAST,
                        0,
  		                NULL, NULL,
  		                g_cclosure_marshal_VOID__VOID,
                 		G_TYPE_NONE, 0);

    signal_value[2]=g_signal_new ("pronounce",
                        G_TYPE_FROM_CLASS (klass),
                        G_SIGNAL_RUN_LAST,
                        0,
  		                NULL, NULL,
  		                g_cclosure_marshal_VOID__VOID,
                 		G_TYPE_NONE, 0);

    signal_value[3]=g_signal_new ("trnextg",
                        G_TYPE_FROM_CLASS (klass),
                        G_SIGNAL_RUN_LAST,
                        0,
  		                NULL, NULL,
  		                g_cclosure_marshal_VOID__VOID,
                 		G_TYPE_NONE, 0);

    signal_value[4]=g_signal_new ("trnext",
                        G_TYPE_FROM_CLASS (klass),
                        G_SIGNAL_RUN_LAST,
                        0,
	                    NULL, NULL,
	                    g_cclosure_marshal_VOID__VOID,
                 		G_TYPE_NONE, 0);

    signal_value[5]=g_signal_new ("pronounce2",
                        G_TYPE_FROM_CLASS (klass),
                        G_SIGNAL_RUN_LAST,
                        0,
  		                NULL, NULL,
  		                g_cclosure_marshal_VOID__VOID,
                 		G_TYPE_NONE, 0);
    

    g_type_class_add_private (gobject_class, sizeof (QuizPadPriv));
}

void
on_delete_clicked(GtkWidget *widget, gpointer data)
{


    QuizPad * qpad=QUIZPAD(data);
    QuizPadPriv  *priv=QUIZPAD_GET_PRIVATE(qpad);

    GString *res=g_string_new("");
    gunichar comp; 
    gunichar *m_word;
	m_word=  g_utf8_to_ucs4(gtk_entry_get_text(GTK_ENTRY(priv->word)), -1, NULL, NULL, NULL);
	gint pos= gtk_editable_get_position(GTK_EDITABLE(priv->word))-1;
    gint p=0;

//g_print("inicio delete\n");
	if(pos==-1)
		return;
//g_print("la pos no es -1\n");
    if(!m_word)
        return;

	if(wcslen((const wchar_t *)m_word)==0)
		return;
	
    for(comp=m_word[p]; comp!=0; p++, comp=m_word[p])
		if(p!=pos)
			g_string_append_unichar(res, comp);		

    gtk_entry_set_text(GTK_ENTRY(priv->word), res->str);

	gtk_editable_set_position(GTK_EDITABLE(priv->word), pos);
//g_print("eliminado\n");

    g_string_free(res,TRUE);
    g_free(m_word);
    
}


void 
on_correct_clicked(GtkWidget *widget, gpointer data)
{
    g_signal_emit(G_OBJECT(data), signal_value[0] ,0);
}

void
on_answer_clicked(GtkWidget *widget, gpointer data)
{
    //g_signal_emit(G_OBJECT(data), signal_value[1] ,0);
    QuizPad * qpad=QUIZPAD(data);
    QuizPadPriv  *priv=QUIZPAD_GET_PRIVATE(qpad);
    gtk_entry_set_text(GTK_ENTRY(priv->word), priv->sanswer->str);
}

void
on_pronounce_clicked(GtkWidget *widget, gpointer data)
{
    g_signal_emit(G_OBJECT(data), signal_value[2] ,0);
}

void
on_pronouncealt_clicked(GtkWidget *widget, gpointer data)
{
    g_signal_emit(G_OBJECT(data), signal_value[5] ,0);
}

void
on_checkm_clicked(GtkWidget *widget, gpointer data)
{

    GtkToggleButton *check=GTK_TOGGLE_BUTTON(widget);
    QuizPad * qpad=QUIZPAD(data);
    QuizPadPriv  *priv=QUIZPAD_GET_PRIVATE(qpad);

     if(gtk_toggle_button_get_active(check))
     {
        if(priv->smeaning)
        {
            if(priv->mode)
                gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(priv->meaning2)),priv->smeaning->str, -1);
            else
            gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(priv->meaning)),priv->smeaning->str, -1);
        }
        
     }
    
    else
    {
        if(priv->mode)
            gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(priv->meaning2)),"", 0);
        else
            gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(priv->meaning)),"", 0);
    }
}


void
on_checkp_clicked(GtkWidget *widget, gpointer data)
{
    GtkToggleButton *check=GTK_TOGGLE_BUTTON(widget);
    QuizPad * qpad=QUIZPAD(data);
    QuizPadPriv  *priv=QUIZPAD_GET_PRIVATE(qpad);

     if(gtk_toggle_button_get_active(check))
     {
        if(priv->spronun)
        {
            if(priv->mode)
                gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(priv->pronun2)),priv->spronun->str, -1);
            else
                gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(priv->pronun)),priv->spronun->str, -1);
        }
        
     }
    
    else
    {
        if(priv->mode)
            gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(priv->pronun2)),"", 0);
        else
            gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(priv->pronun)),"", 0);
    }
}

void
on_checka_clicked(GtkWidget *widget, gpointer data)
{
    GtkToggleButton *check=GTK_TOGGLE_BUTTON(widget);
    QuizPad * qpad=QUIZPAD(data);
    QuizPadPriv  *priv=QUIZPAD_GET_PRIVATE(qpad);

     if(gtk_toggle_button_get_active(check))
     {
        if(priv->sanswer)
            gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(priv->answertv)),priv->sanswer->str, -1);
        
     }
    
    else
        gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(priv->answertv)),"", 0);
}


void
on_nextg_clicked(GtkWidget *widget, gpointer data)
{
    g_signal_emit(G_OBJECT(data), signal_value[3] ,0);
}

void
on_next_clicked(GtkWidget *widget, gpointer data)
{
    g_signal_emit(G_OBJECT(data), signal_value[4] ,0);
}


static void
quiz_pad_init(QuizPad* qpad)
{
    QuizPadPriv  *priv=QUIZPAD_GET_PRIVATE(qpad);


    PangoFontDescription * FontStr=pango_font_description_from_string("serif 25");
    PangoFontDescription * BigFontStr=pango_font_description_from_string("serif 32");

    GtkWidget *hbbuttons=gtk_hbox_new(TRUE, 2); //hbox
    GtkWidget *trhbbuttons=gtk_hbox_new(TRUE, 2); //hbox
    GtkWidget *hbhwpads=gtk_hbox_new(TRUE, 5); //hbox
    GtkWidget *pvb=gtk_vbox_new(FALSE, 10); //vbox
    GtkWidget *trvb=gtk_vbox_new(FALSE, 10); //vbox
    GtkWidget *mbox=gtk_hbox_new(FALSE, 2); //hbox  meaning box
    GtkWidget *pbox=gtk_hbox_new(FALSE, 2); //hbox  pronunciation box
    GtkWidget *mbox2=gtk_hbox_new(FALSE, 2); //hbox  meaning box
    GtkWidget *pbox2=gtk_hbox_new(FALSE, 2); //hbox  pronunciation box
    GtkWidget *abox=gtk_hbox_new(FALSE, 2); //hbox  answer box training mode
    GtkWidget *fbox=gtk_hbox_new(FALSE, 2); //hbox  footer box, it contains a label and a button
    GtkWidget *footer=gtk_frame_new(NULL); //this footer will contain the next group button for training mode
    GtkWidget *bubox=gtk_alignment_new(0.5, 0.5, 0.0, 0.0); //alignment
    GtkWidget *trbubox=gtk_alignment_new(0.5, 0.5, 0.0, 0.0); //alignment
    GtkWidget *bupad=gtk_alignment_new(0.5, 0.5, 0.0, 0.0); //alignment
    GtkWidget *alpad1=gtk_alignment_new(0.5, 0.5, 0.0, 0.0); //alignment
    GtkWidget *alpad2=gtk_alignment_new(0.5, 0.5, 0.0, 0.0); //alignment
    GtkWidget *scrolled_win=gtk_scrolled_window_new(NULL, NULL);
    GtkWidget *scrolled_win2=gtk_scrolled_window_new(NULL, NULL);
    GtkWidget *scrolled_win3=gtk_scrolled_window_new(NULL, NULL);
    GtkWidget *scrolled_win4=gtk_scrolled_window_new(NULL, NULL);
    GtkWidget *scrolled_win5=gtk_scrolled_window_new(NULL, NULL);

    
    priv->qcontainer=pvb;
    priv->trcontainer=trvb;
    g_object_ref(priv->qcontainer);
    g_object_ref(priv->trcontainer);

    
    priv->mode=FALSE;
    priv->del_char=gtk_button_new_with_mnemonic("_delete");//gtk_button_new_with_label("delete");
    priv->correct= gtk_button_new_with_mnemonic("_check");//gtk_button_new_with_label("check");
    priv->answer= gtk_button_new_with_mnemonic("get _answer");//gtk_button_new_with_label("get answer");
    priv->pronounce=gtk_button_new_with_mnemonic("_pronounce");//gtk_button_new_with_label("pronounce");
    priv->pronouncealt=gtk_button_new_with_mnemonic("pronounce a_lt");//gtk_button_new_with_label("pronounce");
    GtkWidget *pronounce2=gtk_button_new_with_mnemonic("_pronounce");//gtk_button_new_with_label("pronounce");
    GtkWidget * pronouncealt2=gtk_button_new_with_mnemonic("pronounce a_lt");//gtk_button_new_with_label("pronounce");
    priv->next=gtk_button_new_with_mnemonic("ne_xt"); // next word button fro training mode
    priv->nextg_button=gtk_button_new_with_mnemonic("_next"); //next button for training mode
    priv->nextg_label=gtk_label_new("When you are ready to pass to the next group click on next button"); //label for footer in training mode
    priv->pad1=tomoe_canvas_new();
    priv->pad2=tomoe_canvas_new();
    //priv->clues=gtk_text_view_new();
    priv->meaning=gtk_text_view_new();
    priv->pronun=gtk_text_view_new();
    priv->meaning2=gtk_text_view_new();
    priv->pronun2=gtk_text_view_new();
    priv->answertv=gtk_text_view_new();
    priv->checkm=gtk_check_button_new_with_mnemonic("show pronun_ciation");
    priv->checkp=gtk_check_button_new_with_mnemonic("show _meaning");
    priv->checkm2=gtk_check_button_new_with_mnemonic("show pronun_ciation");
    priv->checkp2=gtk_check_button_new_with_mnemonic("show _meaning");
    priv->checka=gtk_check_button_new_with_mnemonic("show _answer");    
    priv->word=gtk_entry_new(); 

    //gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->checkm), FALSE);
    //gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->checkp), FALSE);
    gtk_widget_set_size_request(priv->pad1, TOMOE_CANVAS_DEFAULT_SIZE, TOMOE_CANVAS_DEFAULT_SIZE);
    gtk_widget_set_size_request(priv->pad2, TOMOE_CANVAS_DEFAULT_SIZE, TOMOE_CANVAS_DEFAULT_SIZE);
    tomoe_canvas_set_auto_find_time(TOMOE_CANVAS(priv->pad1), AUTO_FIND_TIME);
    tomoe_canvas_set_auto_find_time(TOMOE_CANVAS(priv->pad2), AUTO_FIND_TIME);

    gtk_container_add(GTK_CONTAINER(trbubox), trhbbuttons);
    gtk_container_add(GTK_CONTAINER(bubox), hbbuttons);
    gtk_container_add(GTK_CONTAINER(bupad), hbhwpads);
    gtk_container_add(GTK_CONTAINER(scrolled_win), priv->meaning);
    gtk_container_add(GTK_CONTAINER(scrolled_win2), priv->pronun);
    gtk_container_add(GTK_CONTAINER(scrolled_win3), priv->answertv);
    gtk_container_add(GTK_CONTAINER(scrolled_win4), priv->meaning2);
    gtk_container_add(GTK_CONTAINER(scrolled_win5), priv->pronun2);
    
    
    gtk_signal_connect(GTK_OBJECT(priv->del_char), "clicked", GTK_SIGNAL_FUNC(on_delete_clicked), qpad);
    gtk_signal_connect(GTK_OBJECT(priv->correct), "clicked", GTK_SIGNAL_FUNC(on_correct_clicked), qpad);
    gtk_signal_connect(GTK_OBJECT(priv->answer), "clicked", GTK_SIGNAL_FUNC(on_answer_clicked), qpad);
    gtk_signal_connect(GTK_OBJECT(priv->pronounce), "clicked", GTK_SIGNAL_FUNC(on_pronounce_clicked), qpad);
    gtk_signal_connect(GTK_OBJECT(priv->pronouncealt), "clicked", GTK_SIGNAL_FUNC(on_pronouncealt_clicked), qpad);
    gtk_signal_connect(GTK_OBJECT(pronounce2), "clicked", GTK_SIGNAL_FUNC(on_pronounce_clicked), qpad);
    gtk_signal_connect(GTK_OBJECT(pronouncealt2), "clicked", GTK_SIGNAL_FUNC(on_pronouncealt_clicked), qpad);
    gtk_signal_connect(GTK_OBJECT(priv->checkm), "clicked", GTK_SIGNAL_FUNC(on_checkm_clicked), qpad);
    gtk_signal_connect(GTK_OBJECT(priv->checkp), "clicked", GTK_SIGNAL_FUNC(on_checkp_clicked), qpad);
    gtk_signal_connect(GTK_OBJECT(priv->checkm2), "clicked", GTK_SIGNAL_FUNC(on_checkm_clicked), qpad);
    gtk_signal_connect(GTK_OBJECT(priv->checkp2), "clicked", GTK_SIGNAL_FUNC(on_checkp_clicked), qpad);
    gtk_signal_connect(GTK_OBJECT(priv->checka), "clicked", GTK_SIGNAL_FUNC(on_checka_clicked), qpad);
    gtk_signal_connect(GTK_OBJECT(priv->next), "clicked", GTK_SIGNAL_FUNC(on_next_clicked), qpad);
    gtk_signal_connect(GTK_OBJECT(priv->nextg_button), "clicked", GTK_SIGNAL_FUNC(on_nextg_clicked), qpad);
    gtk_signal_connect(GTK_OBJECT(priv->pad1), "found", GTK_SIGNAL_FUNC(quiz_pad_add_kanji_from_canvas), qpad);
    gtk_signal_connect(GTK_OBJECT(priv->pad2), "found", GTK_SIGNAL_FUNC(quiz_pad_add_kanji_from_canvas), qpad);

    gtk_box_pack_start(GTK_BOX(mbox), scrolled_win, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(mbox), priv->checkm, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(pbox), scrolled_win2, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(pbox), priv->checkp, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(mbox2), scrolled_win4, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(mbox2), priv->checkm2, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(pbox2), scrolled_win5, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(pbox2), priv->checkp2, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(abox), scrolled_win3, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(abox), priv->checka, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbbuttons), priv->del_char, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbbuttons), priv->answer, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbbuttons), priv->correct, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbbuttons), priv->pronounce, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbbuttons), priv->pronouncealt, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(trhbbuttons), pronounce2, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(trhbbuttons), pronouncealt2, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(trhbbuttons), priv->next, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbhwpads), alpad1, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbhwpads), alpad2, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(fbox), priv->nextg_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(fbox), priv->nextg_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(pvb), mbox, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(pvb), pbox, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(pvb), priv->word, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(pvb), bubox, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(pvb), bupad, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(trvb), mbox2, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(trvb), pbox2, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(trvb), abox, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(trvb), trbubox, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(trvb), footer, FALSE, FALSE, 0);
    

    gtk_container_add(GTK_CONTAINER(alpad1), GTK_WIDGET(priv->pad1));
    gtk_container_add(GTK_CONTAINER(alpad2), GTK_WIDGET(priv->pad2));
    gtk_container_add(GTK_CONTAINER(footer), GTK_WIDGET(fbox));

    //gtk_widget_modify_font(priv->clues,BigFontStr);
    gtk_widget_modify_font(priv->meaning,BigFontStr);
    gtk_widget_modify_font(priv->pronun,BigFontStr);
    gtk_widget_modify_font(priv->meaning2,BigFontStr);
    gtk_widget_modify_font(priv->pronun2,BigFontStr);
    gtk_widget_modify_font(priv->answertv,BigFontStr);
    gtk_widget_modify_font(priv->word,FontStr);
    //gtk_text_view_set_justification(GTK_TEXT_VIEW(priv->clues), GTK_JUSTIFY_CENTER);
    gtk_text_view_set_justification(GTK_TEXT_VIEW(priv->meaning), GTK_JUSTIFY_CENTER);
    gtk_text_view_set_justification(GTK_TEXT_VIEW(priv->pronun), GTK_JUSTIFY_CENTER);
    gtk_text_view_set_justification(GTK_TEXT_VIEW(priv->meaning2), GTK_JUSTIFY_CENTER);
    gtk_text_view_set_justification(GTK_TEXT_VIEW(priv->pronun2), GTK_JUSTIFY_CENTER);
    gtk_text_view_set_justification(GTK_TEXT_VIEW(priv->answertv), GTK_JUSTIFY_CENTER);
    //gtk_text_view_set_editable(GTK_TEXT_VIEW(priv->clues), FALSE);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(priv->meaning), FALSE);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(priv->pronun), FALSE);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(priv->meaning2), FALSE);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(priv->pronun2), FALSE);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(priv->answertv), FALSE);
    //gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(priv->clues), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(priv->meaning), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(priv->pronun), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(priv->meaning2), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(priv->pronun2), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(priv->answer), FALSE);
    //gtk_text_view_set_pixels_above_lines(GTK_TEXT_VIEW(priv->clues), 5);
    //gtk_text_view_set_pixels_below_lines(GTK_TEXT_VIEW(priv->clues), 5);
    //gtk_text_view_set_pixels_inside_wrap(GTK_TEXT_VIEW(priv->clues), 5);
    //gtk_text_view_set_left_margin(GTK_TEXT_VIEW(priv->clues), 10);
    //gtk_text_view_set_right_margin(GTK_TEXT_VIEW(priv->clues), 10);

    gtk_container_set_border_width(GTK_CONTAINER(scrolled_win), 1);
    gtk_container_set_border_width(GTK_CONTAINER(scrolled_win2), 1);
    gtk_container_set_border_width(GTK_CONTAINER(scrolled_win3), 1);
    gtk_container_set_border_width(GTK_CONTAINER(scrolled_win4), 1);
    gtk_container_set_border_width(GTK_CONTAINER(scrolled_win5), 1);
    gtk_scrolled_window_set_policy  (GTK_SCROLLED_WINDOW(scrolled_win), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_policy  (GTK_SCROLLED_WINDOW(scrolled_win2), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_policy  (GTK_SCROLLED_WINDOW(scrolled_win3), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_policy  (GTK_SCROLLED_WINDOW(scrolled_win4), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_policy  (GTK_SCROLLED_WINDOW(scrolled_win5), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    

    //gtk_container_add(GTK_CONTAINER(qpad), pvb);


    
    //gtk_widget_show_all(GTK_WIDGET(qpad));
}


static void
quiz_pad_dispose (GObject *object)
{
   // QuizPad *qpad = QUIZPAD (object);
   // QuizPadPriv *priv = QUIZPAD_GET_PRIVATE(qpad);


    if(G_OBJECT_CLASS (quiz_pad_parent_class)->dispose)
        G_OBJECT_CLASS (quiz_pad_parent_class)->dispose (object);
}


QuizPad *
quiz_pad_new (void)
{
    QuizPad *qpad = g_object_new(TYPE_QUIZPAD, NULL);
    return qpad;
}

void
quiz_pad_add_kanji_from_canvas(TomoeCanvas *canvas, gpointer data)
{
    QuizPad *qpad=(QuizPad*)data;
    g_return_if_fail (TOMOE_IS_CANVAS (canvas));
    g_return_if_fail (IS_QUIZPAD (qpad));
    gunichar kanji;

    kanji= (gunichar)tomoe_canvas_get_nth_candidate(canvas, 0, FALSE);
    quiz_pad_insert_kanji(qpad, kanji);
    kanji= (gunichar)tomoe_canvas_get_nth_candidate(canvas, 0, TRUE);
    
    if(kanji!=0)
      quiz_pad_insert_kanji(qpad, kanji);

    tomoe_canvas_clear (canvas);

}

void
quiz_pad_insert_kanji(QuizPad *qpad, gunichar kanji)
{
    QuizPadPriv *priv = QUIZPAD_GET_PRIVATE(qpad);
    g_return_if_fail(priv);
    
    gunichar comp; 
    gunichar *m_word;
    GString *res=g_string_sized_new(8);
	m_word=  g_utf8_to_ucs4(gtk_entry_get_text(GTK_ENTRY(priv->word)), -1, NULL, NULL, NULL);
    gint inc=gtk_editable_get_position(GTK_EDITABLE(priv->word))+1;
    gint p=0;
    gboolean insrt=TRUE;
//g_print("carga de tipos\n");
//g_print("inc tiene valor de: %d\n", inc);
    if(gtk_entry_get_text_length(GTK_ENTRY(priv->word))==0)
        g_string_append_unichar(res, kanji);

    else
    {
        for(comp=m_word[p]; comp!=0; comp=m_word[p])
        {
            if(p==inc-1 && insrt)
            {
	            g_string_append_unichar(res, kanji);
                insrt=FALSE;
            }
            else
            {
                g_string_append_unichar(res, comp);
                p++;
            }

        }
        if(insrt)
            g_string_append_unichar(res, kanji);
    }
    gtk_entry_set_text(GTK_ENTRY(priv->word), res->str);
    gtk_editable_set_position(GTK_EDITABLE(priv->word), inc);
    g_string_free(res, TRUE);
    g_free(m_word);
//g_print("colocación de \n");
}


void 
quiz_pad_insert_clue(QuizPad *qpad, const gchar* pronun, const gchar* meaning, const gchar* answer)
{
    QuizPadPriv *priv = QUIZPAD_GET_PRIVATE(qpad);
    g_return_if_fail(priv);
    

    if(!priv->smeaning)
       priv->smeaning=g_string_new(meaning);
    else
         g_string_assign(priv->smeaning, meaning);

    if(priv->mode)
    {
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->checkm2)))
            gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(priv->meaning2)),priv->smeaning->str, -1);
    }

    else
    {
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->checkm)))
            gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(priv->meaning)),priv->smeaning->str, -1);
    }
    

    if(!priv->spronun)
       priv->spronun=g_string_new(pronun);
    else
        g_string_assign(priv->spronun, pronun);

    if(priv->mode)
    {
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->checkp2)))
            gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(priv->pronun2)),priv->spronun->str, -1);
    }
    else
    {
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->checkp)))
            gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(priv->pronun)),priv->spronun->str, -1);
    }


    if(!priv->sanswer)
       priv->sanswer=g_string_new(answer);
    else
        g_string_assign(priv->sanswer, answer);

    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->checka)))
            gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(priv->answertv)),priv->sanswer->str, -1);


    gtk_entry_set_text(GTK_ENTRY(priv->word), "");
}

void
quiz_pad_clean_checks(QuizPad *qpad)
{
    QuizPadPriv *priv = QUIZPAD_GET_PRIVATE(qpad);
    g_return_if_fail(priv);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->checkm), FALSE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->checkp), FALSE);
}

const gchar*
quiz_pad_get_answer(QuizPad *qpad)
{
    QuizPadPriv *priv = QUIZPAD_GET_PRIVATE(qpad);
    g_return_val_if_fail(priv, NULL);
    
    return gtk_entry_get_text(GTK_ENTRY(priv->word));
}

/*void quiz_pad_set_answer(QuizPad *qpad, const gchar* ans)
{
    QuizPadPriv *priv = QUIZPAD_GET_PRIVATE(qpad);
    g_return_if_fail(priv);

    gtk_entry_set_text(GTK_ENTRY(priv->word), ans);
}*/

void
quiz_pad_replace_frame(QuizPad *qpad, gboolean pad)
{
    QuizPadPriv *priv = QUIZPAD_GET_PRIVATE(qpad);
    g_return_if_fail(priv);

    if(gtk_bin_get_child(GTK_BIN(qpad)))
        gtk_container_remove(GTK_CONTAINER(qpad), gtk_bin_get_child(GTK_BIN(qpad)));

    if(pad)
    {
        gtk_container_add(GTK_CONTAINER(qpad), priv->trcontainer);
        priv->mode = TRUE;
    }
    else
    {
        gtk_container_add(GTK_CONTAINER(qpad), priv->qcontainer);
        priv->mode = FALSE;
    }
    gtk_widget_show_all(GTK_WIDGET(qpad));
}

/*
 * vi:ts=4:nowrap:ai:expandtab
 */
