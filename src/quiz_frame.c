/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */


#include <stdlib.h>
#include <string.h>
#include "quiz_frame.h"



typedef struct _QuizFramePriv QuizFramePriv;


struct _QuizFramePriv
{
    Question *que;	
	QuizPad *qp;
	FinishFrame *ff;
	ControlFrame *cf;
	MessageFrame *mf;
	sqlite3 *db;
	gint total; //number of total questions
	gint correct; //correct questions
	gint showed;
    gboolean mode; //FALSE = Quiz mode  / TRUE = trainig mode

};


#define QUIZFRAME_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TYPE_QUIZFRAME, QuizFramePriv))

G_DEFINE_TYPE (QuizFrame, quiz_frame, GTK_TYPE_WINDOW)

#define QFRAME_SIZE_STR "gui.main.size"
#define TABLE_ST "create table 'quiz' (id INTEGER PRIMARY KEY AUTOINCREMENT, kanji TEXT, pronuntiation TEXT, meaning TEXT, selected INTEGER);"
#define TABLE_TR "create table 'training' (id INTEGER PRIMARY KEY AUTOINCREMENT, kanji TEXT, pronuntiation TEXT, meaning TEXT, selected INTEGER);"
#define TABLE_CLEAN "delete from quiz;"
#define TABLE_CLEANTR "delete from training;"
#define AUTO_INC_R "delete from sqlite_sequence where name='quiz';"
#define AUTO_INC_RTR "delete from sqlite_sequence where name='training';"
#define TABLE_INSERT "insert into %s (kanji, pronuntiation, meaning, selected) values ('%s', '%s','%s',0);"
#define GET_ROW "SELECT id, kanji, pronuntiation, meaning  FROM quiz WHERE selected=0 ORDER BY RANDOM() LIMIT 1;"
#define GET_ROWNR "SELECT id, kanji, pronuntiation, meaning  FROM quiz WHERE selected=0 LIMIT 1;" //no random
#define GET_ROWUNTR "SELECT id, kanji, pronuntiation, meaning  FROM training WHERE selected=0 ORDER BY RANDOM() LIMIT 1;" //get row unselected from training table
#define GET_ROWTR "SELECT id, kanji, pronuntiation, meaning  FROM training ORDER BY RANDOM() LIMIT 1;"  //get row 
#define RESET_SELECTED "UPDATE quiz SET selected=0;"
#define PRON_EXPR "http://translate.google.com/translate_tts?tl=ja&ie=utf-8&q=%s"
#define PRON_EXPR2 "http://assets.languagepod101.com/dictionary/japanese/audiomp3.php?kanji=%s&kana=%s"


/* virtual functions for GtkObject class */
static void   quiz_frame_dispose          (GObject           *object);
void          setup_handlers              (QuizFrame         *qframe);
void          update_selected             (QuizFrame *qframe, char *id,const gchar * table);
void          training_next_group         (QuizFrame *qframe);


static void
quiz_frame_class_init(QuizFrameClass* klass)
{
    GObjectClass   *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->dispose             = quiz_frame_dispose;
    g_type_class_add_private (gobject_class, sizeof (QuizFramePriv));
}



static void
quiz_frame_init(QuizFrame* qframe)
{
    QuizFramePriv  *priv=QUIZFRAME_GET_PRIVATE(qframe);
    gint res;
    char *re;

    priv->qp=quiz_pad_new();
	priv->ff=finish_frame_new();
	priv->cf=control_frame_new();
	priv->mf=message_frame_new();
    priv->que=question_new ();
    priv->que->id=g_string_sized_new(3);
    priv->que->kanji=g_string_sized_new(50);
    priv->que->pron=g_string_sized_new(50);
    priv->que->meaning=g_string_sized_new(50);
    priv->total=0;
    g_object_ref(priv->qp);
    g_object_ref(priv->ff);
    g_object_ref(priv->cf);
    g_object_ref(priv->mf);

    gtk_window_set_title(GTK_WINDOW(qframe), "J-Quiz --prototipo funcional--");
    //gtk_window_set_policy (GTK_WINDOW (qframe), FALSE, FALSE, TRUE);

    setup_handlers(qframe);

    res=sqlite3_open(":memory:", &priv->db);
	if( res )
    {
		g_print("Can't open database: \n");
		sqlite3_close(priv->db);
		g_error("A ocurrido un problema en la apertura de la base de datos el programa no puede continuar\n");
  	}


    sqlite3_exec(priv->db, TABLE_ST, NULL, 0, &re);
    sqlite3_exec(priv->db, TABLE_TR, NULL, 0, &re); //same tables

    control_frame_replace_frame (priv->cf, FALSE);
    gtk_container_add(GTK_CONTAINER(qframe), GTK_WIDGET(priv->cf));
    gtk_widget_show_all(GTK_WIDGET(qframe));
}

static void
quiz_frame_dispose (GObject *object)
{

    QuizFrame *qframe = QUIZFRAME (object);
    QuizFramePriv *priv = QUIZFRAME_GET_PRIVATE(qframe);

    g_object_unref((gpointer)gtk_bin_get_child(GTK_BIN(qframe)));
    g_object_unref((gpointer)priv->qp);
    g_object_unref((gpointer)priv->ff);
    g_object_unref((gpointer)priv->cf);
    g_object_unref((gpointer)priv->mf);
	sqlite3_close(priv->db);
    question_free (priv->que);
    if(G_OBJECT_CLASS (quiz_frame_parent_class)->dispose)
    G_OBJECT_CLASS (quiz_frame_parent_class)->dispose (object);
}

QuizFrame *
quiz_frame_new (void)
{
    QuizFrame *qframe = g_object_new(TYPE_QUIZFRAME, NULL);
    return qframe;
}


void
replace(QuizFrame *qframe, GtkWidget *widget)
{
    gtk_container_remove(GTK_CONTAINER(qframe), gtk_bin_get_child(GTK_BIN(qframe)));
	gtk_container_add(GTK_CONTAINER(qframe), GTK_WIDGET(widget));
}

void
show_control_open_start(QuizFrame *qframe)
{
    QuizFramePriv  *priv=QUIZFRAME_GET_PRIVATE(qframe);

    g_return_if_fail(priv);

    //gtk_window_set_title(GTK_WINDOW(qframe), "J-Quiz --prototipo funcional--");
    g_print("resultado de total es: %d",priv->total);
    if(!priv->mode)
    {
	    if(priv->total!=0)
            control_frame_set_quiz_sensitive(priv->cf, TRUE);
	    else
            control_frame_set_quiz_sensitive(priv->cf, FALSE);
    }
    control_frame_replace_frame(CONTROLFRAME(priv->cf), FALSE);
    g_print("cambio de frame");
    replace(qframe, GTK_WIDGET(priv->cf));
	gtk_widget_show_all(GTK_WIDGET(qframe));
}

void
on_show_control_open_start(GtkWidget *widget, gpointer data)
{
    QuizFrame *qframe=QUIZFRAME(data);
    show_control_open_start(qframe);
}

void
show_control_quiz_train(QuizFrame *qframe)
{
    QuizFramePriv  *priv=QUIZFRAME_GET_PRIVATE(qframe);

    g_return_if_fail(priv);

    //gtk_window_set_title(GTK_WINDOW(qframe), "J-Quiz --prototipo funcional--");

    control_frame_replace_frame(CONTROLFRAME(priv->cf), TRUE);

    replace(qframe, GTK_WIDGET(priv->cf));
	gtk_widget_show_all(GTK_WIDGET(qframe));
}

void 
correct_answer(GtkWidget *widget, gpointer data)
{
    QuizFrame *qframe=QUIZFRAME(data);
    QuizFramePriv  *priv=QUIZFRAME_GET_PRIVATE(qframe);

    g_return_if_fail(priv);    
    const gchar *ans=quiz_pad_get_answer(priv->qp);
	gboolean next;		
		
	if(priv->showed==priv->total)
		next=FALSE;
	else
		next=TRUE;

	if(g_utf8_collate(ans,priv->que->kanji->str)==0)
	{
		
        message_frame_set(priv->mf, TRUE, priv->que->kanji->str, ans, next);
		(priv->correct)+=1;
	}
	else
		message_frame_set(priv->mf, FALSE, priv->que->kanji->str, ans, next);

    //gtk_container_remove(GTK_CONTAINER(qframe), gtk_bin_get_child(GTK_BIN(qframe)));
	//gtk_container_add(GTK_CONTAINER(qframe), GTK_WIDGET(priv->mf));
    replace(qframe, GTK_WIDGET(priv->mf));
	//show_all_children(false);
    gtk_widget_show_all(GTK_WIDGET(qframe));
}

/*void 
set_answer(GtkWidget *widget, gpointer data)
{
    QuizFrame *qframe=QUIZFRAME(data);
    QuizFramePriv  *priv=QUIZFRAME_GET_PRIVATE(qframe);

    g_return_if_fail(priv);
    quiz_pad_set_answer(priv->qp, priv->que->kanji->str);
}*/

void pronounce(GtkWidget *widget, gpointer data)
{
    QuizFrame *qframe=QUIZFRAME(data);
    QuizFramePriv  *priv=QUIZFRAME_GET_PRIVATE(qframe);

    g_return_if_fail(priv);

    GString *line=g_string_sized_new(sizeof(PRON_EXPR)+15);

    g_string_printf (line,PRON_EXPR, priv->que->kanji->str);
    g_print("%s\n", line->str);

    gst_element_set_state (pipeline, GST_STATE_NULL);
    g_object_set (G_OBJECT (source), "location", line->str, NULL);

    gst_element_set_state (pipeline, GST_STATE_PLAYING);
    g_main_loop_run (loop);



    g_string_free(line, TRUE);
}

void pronounce2(GtkWidget *widget, gpointer data)
{
    QuizFrame *qframe=QUIZFRAME(data);
    QuizFramePriv  *priv=QUIZFRAME_GET_PRIVATE(qframe);

    g_return_if_fail(priv);

    gchar *kanji=g_uri_escape_string(priv->que->kanji->str, NULL, FALSE);
    gchar *meaning=g_uri_escape_string(priv->que->meaning->str, NULL, FALSE);
    GString *line=g_string_sized_new(sizeof(PRON_EXPR2)+15);

    g_string_printf (line,PRON_EXPR2, kanji, meaning);
    g_print("%s\n", line->str);

    gst_element_set_state (pipeline, GST_STATE_NULL);
    g_object_set (G_OBJECT (source), "location", line->str, NULL);

    gst_element_set_state (pipeline, GST_STATE_PLAYING);
    g_main_loop_run (loop);



    g_string_free(line, TRUE);
    if(kanji)
        g_free(kanji);
    if(meaning)
        g_free(meaning);
}

void
sql_error(char *error)
{
    g_warning("SQL_ERROR: %s", error);
    sqlite3_free(error);
}


gint
count_records(QuizFrame *qframe, gboolean training) //FALSE quiz, TRUE training
{
    QuizFramePriv *priv = QUIZFRAME_GET_PRIVATE(qframe);
    char *expr;

    if(training)
        expr="select count(*) from training";
    else
        expr="select count(*) from quiz";

    char **result;
	sqlite3_get_table(
	  priv->db,          /* An open database */
	  expr,     /* SQL to be evaluated */
	  &result,    /* Results of the query */
	  NULL,           /* Number of result rows written here */
	  NULL,        /* Number of result columns written here */
	  NULL      /* Error msg written here */
	);
    
    priv->total=(gint)atoi(result[1]);
    sqlite3_free_table(result);
    return priv->total;
}

gint
get_question(QuizFrame *qframe)
{
    QuizFramePriv *priv = QUIZFRAME_GET_PRIVATE(qframe);

   
	char** result;
	int rows=0;
    gboolean mode = priv->mode;

    if(mode)
        sqlite3_get_table(
		  priv->db,          /* An open database */
		  GET_ROWUNTR,     /* SQL to be evaluated */
		  &result,    /* Results of the query */
		  &rows,           /* Number of result rows written here */
		  NULL,        /* Number of result columns written here */
		  NULL       /* Error msg written here */
		);
    else
	    sqlite3_get_table(
		      priv->db,          /* An open database */
		      GET_ROW,     /* SQL to be evaluated */
		      &result,    /* Results of the query */
		      &rows,           /* Number of result rows written here */
		      NULL,        /* Number of result columns written here */
		      NULL       /* Error msg written here */
		    );
	if(rows==0)
    {
        sqlite3_free_table(result);
        if(mode)
        {
            //in traning mode the priority is to show all the rows, in this stage the rows of the group have been showed
            //so it will only choose a random row

            sqlite3_get_table(
              priv->db,          /* An open database */
              GET_ROWTR,     /* SQL to be evaluated */
              &result,    /* Results of the query */
              &rows,           /* Number of result rows written here */
              NULL,        /* Number of result columns written here */
              NULL       /* Error msg written here */
            ); 
        }
        else
		    return rows; //0
    }


    question_set(priv->que, result[4], result[5], result[6], result[7]);
    if(mode)
    {
        update_selected(qframe, result[4], "training");
    }
    else
    {
	    update_selected(qframe, result[4], "quiz");
        (priv->showed)+=1;
    }
	
    sqlite3_free_table(result);
	return rows; //pos: it must return 1 if a row was found otherwise it must return 0
	

}

void
show_question(QuizFrame *qframe)
{

    QuizFramePriv *priv = QUIZFRAME_GET_PRIVATE(qframe);



	if(get_question(qframe)==1)
	{	

        //GError *perror;
        //gchar * pal=g_filename_to_uri(priv->que->meaning->str,NULL,NULL);

        g_print("OHHHHH\n");
        g_print("meaning dice: %s\n",priv->que->meaning->str);
        quiz_pad_insert_clue(priv->qp, priv->que->pron->str, priv->que->meaning->str, priv->que->kanji->str);
        g_print("show inicio\n");		
		replace(qframe, GTK_WIDGET(priv->qp));
		gtk_widget_show_all(GTK_WIDGET(qframe));
        g_print("show fin\n");
		return;
	}
	else //no more questions
	{
        finish_frame_set(priv->ff, priv->correct, priv->total);		
		replace(qframe, GTK_WIDGET(priv->ff));
        gtk_widget_show_all(GTK_WIDGET(qframe));
	}
}

void
on_show_question(GtkWidget *widget, gpointer data)
{
    QuizFrame *qframe=QUIZFRAME(data);
    show_question(qframe);
}

void
show_quiz(QuizFrame *qframe)
{
    QuizFramePriv *priv = QUIZFRAME_GET_PRIVATE(qframe);
    g_return_if_fail(priv);
    priv->mode = FALSE;

    priv->showed=0;
    priv->correct=0;

    quiz_pad_clean_checks(priv->qp);
    quiz_pad_replace_frame(priv->qp, FALSE);
    show_question(qframe);
    g_print("show quiz\n");
}

void
restart_quiz(GtkWidget *widget, gpointer data)
{
    QuizFrame *qframe=QUIZFRAME(data);
    QuizFramePriv *priv = QUIZFRAME_GET_PRIVATE(qframe);
	//show_quiz(qframe, TRUE);
    sqlite3_exec(priv->db, RESET_SELECTED, NULL, 0, NULL);
    show_control_quiz_train(qframe);
}

void 
update_selected(QuizFrame *qframe, char *id, const gchar * table)
{
	GString *st=g_string_sized_new(50);
    QuizFramePriv *priv = QUIZFRAME_GET_PRIVATE(qframe);
    
    g_return_if_fail(id);
    g_return_if_fail(priv);
    if(!table)
        return;
	
    g_string_printf (st,"update %s set selected=1 where id=%s",table, id);

	sqlite3_exec(priv->db, st->str, NULL, 0, NULL);
    g_string_free(st, TRUE);
    g_print("hey blondy\n");
}

void
open_file(GtkWidget *widget, gpointer data)
{

    QuizFrame *qframe=QUIZFRAME(data);
    QuizFramePriv *priv = QUIZFRAME_GET_PRIVATE(qframe);

    GError *ferror=NULL;
    char *sqlerror=NULL;
    gchar *file=control_frame_get_filename(priv->cf);
    GIOChannel*  input=g_io_channel_new_file(file, "r", &ferror);
    GString *line=g_string_sized_new(50);
    GIOStatus status;
    
    if(!input)
    {
        g_warning("ERROR: Could not open the file %s due to: %s", file, ferror->message);
        g_error_free(ferror);
        return;
    }

    sqlite3_exec(priv->db, TABLE_CLEAN, NULL, 0, &sqlerror);
    
    if(sqlerror)
        sql_error(sqlerror);
    
	sqlite3_exec(priv->db, AUTO_INC_R, NULL, 0, &sqlerror);

    if(sqlerror)
        sql_error(sqlerror);


    status=g_io_channel_read_line_string(input, line, NULL, &ferror);
    for(; status!=G_IO_STATUS_EOF; status=g_io_channel_read_line_string(input, line, NULL, &ferror))
    {
        gchar **words=NULL;
        gint size; 
        gchar *nl;
        nl=strchr(line->str,'\n');
        if(nl)
        {
            *nl='\0';
        }
        words=g_strsplit(line->str, "\t", -1);
        
        if(words==NULL)
        {
            g_warning("WARNING: A line from this file is empty\n");
            continue;
        }

        for(size=0; words[size];size++);
        //size--;

        if(size<3)
        {
            g_warning("WARNING: were found less than 3 words in a line: %s\n ", line->str);
            continue;
        }

        if(size>3)
        {
            g_warning("WARNING: were found more than 3 words in a line this program only supports 3 words/groups\n");
        }
        g_print("%s, %s, %s\n", words[0], words[1], words[2]);
        g_string_printf (line,TABLE_INSERT, "quiz", words[0], words[1], words[2]);
        
        sqlite3_exec(priv->db, line->str, NULL, 0, &sqlerror);
        g_print("Agregado a la base de datos\n");
        gtk_window_set_title(GTK_WINDOW(qframe), file);
        
        if(sqlerror)
            sql_error(sqlerror);
        
        g_strfreev(words);
        if(ferror)
            g_error_free(ferror);
        
    }
    g_string_free(line, TRUE);
    count_records(qframe, FALSE);
    //show_quiz(qframe, FALSE);
    show_control_quiz_train(qframe);
    g_print("terminada\n");
    
}

void
quiz_mode(GtkWidget *widget, gpointer data)
{
    /*QuizFrame *qframe=QUIZFRAME(data);
    QuizFramePriv  *priv=QUIZFRAME_GET_PRIVATE(qframe);
    priv->mode = FALSE; //FALSE = Quiz mode
    control_frame_replace_frame (priv->cf, FALSE);
    show_question(qframe);*/
    QuizFrame *qframe=QUIZFRAME(data);
    show_quiz(qframe);

}

void
training_mode(GtkWidget *widget, gpointer data)
{
    QuizFrame *qframe=QUIZFRAME(data);
    QuizFramePriv  *priv=QUIZFRAME_GET_PRIVATE(qframe);
    priv->mode = TRUE; //TRUE = training mode
    quiz_pad_replace_frame(priv->qp, TRUE);
    training_next_group(qframe);
    show_question(qframe);

}


void
on_training_next_group(GtkWidget *widget, gpointer data)
{
    QuizFrame *qframe=QUIZFRAME(data);
    training_next_group(qframe);
}

void
training_next_group(QuizFrame *qframe)
{
    QuizFramePriv  *priv=QUIZFRAME_GET_PRIVATE(qframe);
    gint gnum=control_frame_get_gnumber(priv->cf);
    gboolean gorder = control_frame_get_order(priv->cf);
    char *sqlerror=NULL;
    gint count = 0;
    char** result;
    int rows=1;
    GString *line=g_string_sized_new(50);


    sqlite3_exec(priv->db, TABLE_CLEANTR, NULL, 0, &sqlerror);
    
    if(sqlerror)
        sql_error(sqlerror);
    
	sqlite3_exec(priv->db, AUTO_INC_RTR, NULL, 0, &sqlerror);

    if(sqlerror)
        sql_error(sqlerror);


    for(; count<gnum && rows; count++)
    {

        if(gorder)
            sqlite3_get_table(
                priv->db,          /* An open database */
                GET_ROWNR,     /* SQL to be evaluated */
                &result,    /* Results of the query */
                &rows,           /* Number of result rows written here */
                NULL,        /* Number of result columns written here */
                NULL       /* Error msg written here */
           );
        else
            sqlite3_get_table(
	          priv->db,          /* An open database */
	          GET_ROW,     /* SQL to be evaluated */
	          &result,    /* Results of the query */
	          &rows,           /* Number of result rows written here */
	          NULL,        /* Number of result columns written here */
	          NULL       /* Error msg written here */
	        );

        if(rows)
        {
            update_selected(qframe, result[4], "quiz");
            g_string_printf (line,TABLE_INSERT, "training", result[5], result[6], result[7]);
            sqlite3_exec(priv->db, line->str, NULL, 0, &sqlerror);
            if(sqlerror)
                sql_error(sqlerror);
        }
        sqlite3_free_table(result);

    }

    if(count_records(qframe, TRUE))
        show_question(qframe);

    else
    {
        g_print("es cero\n");
        show_control_open_start(qframe);
    }
    g_string_free(line, TRUE);



    
}

void 
setup_handlers(QuizFrame* qframe)
{

    QuizFramePriv  *priv=QUIZFRAME_GET_PRIVATE(qframe);

    gtk_signal_connect(GTK_OBJECT(priv->cf), "open", GTK_SIGNAL_FUNC(open_file), (gpointer)qframe);
    gtk_signal_connect(GTK_OBJECT(priv->cf), "quiz", GTK_SIGNAL_FUNC(restart_quiz), (gpointer)qframe);
    gtk_signal_connect(GTK_OBJECT(priv->cf), "quizm", GTK_SIGNAL_FUNC(quiz_mode), (gpointer)qframe);
    gtk_signal_connect(GTK_OBJECT(priv->cf), "trainm", GTK_SIGNAL_FUNC(training_mode), (gpointer)qframe);
    gtk_signal_connect(GTK_OBJECT(priv->qp), "correct", GTK_SIGNAL_FUNC(correct_answer), (gpointer)qframe);
    //gtk_signal_connect(GTK_OBJECT(priv->qp), "answer", GTK_SIGNAL_FUNC(set_answer), (gpointer)qframe);
    gtk_signal_connect(GTK_OBJECT(priv->qp), "pronounce", GTK_SIGNAL_FUNC(pronounce), (gpointer)qframe);
    gtk_signal_connect(GTK_OBJECT(priv->qp), "pronounce2", GTK_SIGNAL_FUNC(pronounce2), (gpointer)qframe);
    gtk_signal_connect(GTK_OBJECT(priv->qp), "trnext", GTK_SIGNAL_FUNC(on_show_question), (gpointer)qframe);
    gtk_signal_connect(GTK_OBJECT(priv->qp), "trnextg", GTK_SIGNAL_FUNC(on_training_next_group), (gpointer)qframe);
    gtk_signal_connect(GTK_OBJECT(priv->mf), "scontinue", GTK_SIGNAL_FUNC(on_show_question), (gpointer)qframe);
    gtk_signal_connect(GTK_OBJECT(priv->ff), "finish", GTK_SIGNAL_FUNC(on_show_control_open_start), (gpointer)qframe);
}


Question     
*question_new ( )
{
    Question *q = g_new (Question, 1);

    g_return_val_if_fail (q, NULL);


    return q;
}

Question *
question_copy (const Question *question)
{
    Question *new_question;

    g_return_val_if_fail (question, NULL);

    new_question = g_new (Question, 1);
    *new_question = *question;
    return new_question;
}

void
question_free (Question *question)
{
    g_return_if_fail (question);

    g_string_free(question->id, TRUE);
    g_string_free(question->kanji, TRUE);
    g_string_free(question->pron, TRUE);
    g_string_free(question->meaning, TRUE);

    g_free (question);
}

GType
question_get_type (void)
{
    static GType our_type = 0;

    if (our_type == 0) {
        const gchar *str;
#if GLIB_CHECK_VERSION(2, 10, 0)
        str = g_intern_static_string ("Question");
#else
        str = "Question";
#endif
        our_type =
            g_boxed_type_register_static (str,
                                          (GBoxedCopyFunc)question_copy,
                                          (GBoxedFreeFunc)question_free);
    }

    return our_type;
}


void
question_set(Question *question, const gchar *id, const gchar *kanji, const gchar *pron, const gchar *meaning)
{

    //g_print("el meaning original dice:%s\n",meaning);
    g_return_if_fail (question);
    g_string_assign(question->id, id);
    g_string_assign(question->kanji, kanji);
    g_string_assign(question->pron, pron);
    g_string_assign(question->meaning, meaning);
    //g_print("el meaning copiado es: %s\n", question->meaning->str);

}

/*
 * vi:ts=4:nowrap:ai:expandtab
 */
