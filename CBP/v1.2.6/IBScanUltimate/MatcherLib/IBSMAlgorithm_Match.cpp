/* *************************************************************************************************
 * IBAlgorithm_Match.cpp
 *
 * DESCRIPTION:
 *     Template matching for IBScanMatcher library
 *     http://www.integratedbiometrics.com
 *
 * NOTES:
 *     Copyright (c) Integrated Biometrics, 2013
 *
 * HISTORY:
 ************************************************************************************************ */
#include "stdafx.h"
#include "IBSMAlgorithm.h"
#include <math.h>
#ifdef _WINDOWS
#else
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#endif

int CIBSMAlgorithm::_Algo_SetMatchingLevel(int matching_level)
{
    switch(matching_level)
    {
        case 1: m_MatchingThreshold = 1600; break;
        case 2: m_MatchingThreshold = 2000; break;
        case 3: m_MatchingThreshold = 2400; break;
        case 4: m_MatchingThreshold = 2800; break;
        case 5: m_MatchingThreshold = 3200; break;
        case 6: m_MatchingThreshold = 3600; break;
        case 7: m_MatchingThreshold = 4000; break;
        default: return -1;
    }

    m_MatchingLevel = matching_level;

    return matching_level;
}

int CIBSMAlgorithm::_Algo_GetMatchingLevel()
{
    return m_MatchingLevel;
}

void CIBSMAlgorithm::_ALGO_MatchingInit()
{
    int i;
    int rLim, angleLim;

    for( i=0; i<MAX_DIAGONAL_LENGTH; i++ )
    {
        rLim = (int)(rMin + rInc*i);
		if(i==0)
			angleLim = (int)angleMin;
		else
			angleLim = (int)(angleMin + rLim*(180./pi/(float)i));

        m_RLimForMatching[i] = rLim;
        if(angleLim > 255)
            angleLim = 255;

        if(i<24) angleLim = 21;
        m_AngleLimForMatching[i] = (unsigned char)angleLim;
        m_RpRForMatching[i] = (unsigned short)(P_rMax/rLim);
        m_RpAngleForMatching[i] = (unsigned short)(P_rMax/angleLim);

//      arrayANGLELIM[i] = angleLim;
//      arrayRLIM[i] = rLim;
//      arrayRP_R[i] = P_rMax/rLim;
//      arrayRP_ANGLE[i] = P_rMax/angleLim;
    }
}

int CIBSMAlgorithm::_ALGO_Matching(FeatureVector* p_feavector_db, FeatureVector* p_feavector_in)
{
    int Ref,Inp,SCORE,LOOP_COUNT=0,j,k;
    int matched_pair_COUNT=0;
    int TRACING_GOING=1;
    int STEP_MATCEDMINUTIAE_COUNT=0;
    int TRACING_SAVE_COUNT=0,SAVE;
    int DB,INPUT;
    int Prev_Ref,Prev_Inp;
    int db_index,in_index,count,ii,Total_NMS;
    int MAX_SCORE=0,MAX_INDEX=0;
    int remove_i;
#ifdef GLOBAL_MATCHING
    int Prev_Score,Diff_angle,Diff_thetta;
#endif

    int Count_Candidate,i;

    CANDIDATE_Pair  g_Candidate_Pair;
    char matched_pair_DB[MAX_MINUTIAE];
    char matched_pair_IN[MAX_MINUTIAE];
    char TRACING_STACK_DB[MAX_MINUTIAE];
    char TRACING_STACK_IN[MAX_MINUTIAE];
    unsigned short matched_pair_SCORE[MAX_MINUTIAE];

    int MATNEIGBOR8_DB[D_NEIGHBOR], MATNEIGBOR8_IN[D_NEIGHBOR], MATNEIGBOR8_COUNT;
    short overlap_minutiae_db[MAX_MINUTIAE], overlap_minutiae_in[MAX_MINUTIAE];

#if (__SCT_PRODUCT_TYPE__ == __SCT_PRODUCT_MODULE__ && defined(__G_DEBUG__))
unsigned int speed_check;
dev_StartTimeCheck();
#endif

    //Step 1.  Searching Candidate Minutiae
    Count_Candidate = _SearchCandidateMinutiae(p_feavector_db, p_feavector_in, &g_Candidate_Pair);

#if (__SCT_PRODUCT_TYPE__ == __SCT_PRODUCT_MODULE__ && defined(__G_DEBUG__))
speed_check = dev_GetTimeCheck();
dev_StopTimeCheck();
dev_StartTimeCheck();
#endif

    if(Count_Candidate <= 0)
        return 0;

    memset(m_StepMatchedMinutiaeMatchedCount,0,sizeof(m_StepMatchedMinutiaeMatchedCount));
    memset(m_StepMatchedMinutiaeDB,-1,sizeof(m_StepMatchedMinutiaeDB));
    memset(m_StepMatchedMinutiaeIN,-1,sizeof(m_StepMatchedMinutiaeIN));
    memset(m_StepMatchedMinutiaeScore,0,sizeof(m_StepMatchedMinutiaeScore));
    memset(m_StepMatchedMinutiaeScoreMinutiae,0,sizeof(m_StepMatchedMinutiaeScoreMinutiae));

    for(i=0; i<Count_Candidate; i++)
    {
        memset(TRACING_STACK_DB,-1,sizeof(TRACING_STACK_DB));
        memset(TRACING_STACK_IN,-1,sizeof(TRACING_STACK_IN));
        memset(matched_pair_DB,-1,sizeof(matched_pair_DB));
        memset(matched_pair_IN,-1,sizeof(matched_pair_IN));
        memset(matched_pair_SCORE,0,sizeof(matched_pair_SCORE));
        matched_pair_COUNT=0;
        TRACING_SAVE_COUNT=0;

        Prev_Ref=-1;
        Prev_Inp=-1;
        Ref = g_Candidate_Pair.Ref_Minutiae[i];
        Inp = g_Candidate_Pair.Inp_Minutiae[i];

        LOOP_COUNT=0;
        TRACING_GOING=1;
        while(TRACING_GOING)
        {
#ifdef GLOBAL_MATCHING
            Diff_angle=0;
            Diff_thetta=0;
            if(Prev_Ref>=0 && Prev_Inp>=0)
                Prev_Score = _Judge_TracingTree(Prev_Ref, Ref, Prev_Inp, Inp, p_feavector_db, p_feavector_in, &Diff_angle,&Diff_thetta);
            else
                Prev_Score = -1;
            SCORE = _Neighbor_Matching_Calculation_New(Ref,Inp,p_feavector_db,p_feavector_in, MATNEIGBOR8_DB, MATNEIGBOR8_IN, &MATNEIGBOR8_COUNT);
            if(SCORE>0 && (Prev_Score<0 || Prev_Score>0) && MATNEIGBOR8_COUNT>1)
#else
            SCORE = _Neighbor_Matching_Calculation_New(Ref,Inp,p_feavector_db,p_feavector_in, MATNEIGBOR8_DB, MATNEIGBOR8_IN, &MATNEIGBOR8_COUNT);
            if(SCORE>0)
#endif
            {
                matched_pair_DB[matched_pair_COUNT]=Ref;
                matched_pair_IN[matched_pair_COUNT]=Inp;
                matched_pair_SCORE[matched_pair_COUNT]=SCORE;
                matched_pair_COUNT++;
                for(j=0; j<MATNEIGBOR8_COUNT; j++)
                {
                    DB=MATNEIGBOR8_DB[j];
                    INPUT=MATNEIGBOR8_IN[j];
                    SAVE=0;
                    for(k=0; k<matched_pair_COUNT; k++)
                    {
                        if(DB == matched_pair_DB[k] || INPUT == matched_pair_IN[k])
                            SAVE=1;
                    }
                    if(SAVE==0)
                    {
                        for(k=0; k<TRACING_SAVE_COUNT; k++)
                        {
                            if(DB == TRACING_STACK_DB[k] || INPUT == TRACING_STACK_IN[k])
                                SAVE=1;
                        }
                    }
                    if(SAVE==0)
                    {
                        TRACING_STACK_DB[TRACING_SAVE_COUNT] = DB;
                        TRACING_STACK_IN[TRACING_SAVE_COUNT] = INPUT;
                        TRACING_SAVE_COUNT++;
                    }
                }
            }
            if(LOOP_COUNT>=TRACING_SAVE_COUNT)
                TRACING_GOING=0;
            else
            {
                Prev_Ref=Ref;
                Prev_Inp=Inp;
                Ref = TRACING_STACK_DB[LOOP_COUNT];
                Inp = TRACING_STACK_IN[LOOP_COUNT];
                LOOP_COUNT++;
                for(j=0; j<matched_pair_COUNT; j++)
                {
                    if(Ref == matched_pair_DB[j] || Inp == matched_pair_IN[j])
                    {
                        Ref = TRACING_STACK_DB[LOOP_COUNT];
                        Inp = TRACING_STACK_IN[LOOP_COUNT];
                        LOOP_COUNT++;
                        j=0;
                    }
                    if(LOOP_COUNT>TRACING_SAVE_COUNT)
                    {
                        TRACING_GOING=0;
                        break;
                    }
                }
            }
        }
//      if(matched_pair_COUNT<=0)continue;
        //중복 미뉴샤 제거
        memset(overlap_minutiae_db,0,sizeof(overlap_minutiae_db));
        memset(overlap_minutiae_in,0,sizeof(overlap_minutiae_in));
        for(remove_i=0; remove_i<matched_pair_COUNT; remove_i++)
        {
            db_index = matched_pair_DB[remove_i];
            overlap_minutiae_db[db_index] += 1;
            in_index = matched_pair_IN[remove_i];
            overlap_minutiae_in[in_index] += 1;
        }
        //DB네이버 중복 삭제
        for(remove_i=0; remove_i<MAX_MINUTIAE; remove_i++)
        {
            count = overlap_minutiae_db[remove_i];
            if(count<2) continue;

            MAX_SCORE=0;
            for(ii=0; ii<matched_pair_COUNT; ii++)
            {
                db_index = matched_pair_DB[ii];
                if(remove_i==db_index)
                {
                    SCORE =matched_pair_SCORE[ii];
                    if(SCORE>MAX_SCORE)
                    {
                        MAX_SCORE=SCORE;
                        MAX_INDEX=ii;
                    }
                }
            }
            for(ii=0; ii<matched_pair_COUNT; ii++)
            {
                db_index = matched_pair_DB[ii];
                if(remove_i==db_index && ii!=MAX_INDEX)
                {
                    matched_pair_SCORE[ii]=0;
//                  matched_pair_DB[ii]=-1;
//                  matched_pair_IN[ii]=-1;
                }
            }
        }
        //INPUT네이버 중복 삭제
        // bug fixed
        for(remove_i=0; remove_i<MAX_MINUTIAE; remove_i++)  // for(remove_i=0; remove_i<D_NEIGHBOR; remove_i++)
        {
            count = overlap_minutiae_in[remove_i];
            if(count<2) continue;

            MAX_SCORE=0;
            for(ii=0; ii<matched_pair_COUNT; ii++)
            {
                in_index = matched_pair_IN[ii];
                if(remove_i==in_index)
                {
                    SCORE =matched_pair_SCORE[ii];
                    if(SCORE>MAX_SCORE)
                    {
                        MAX_SCORE=SCORE;
                        MAX_INDEX=ii;
                    }
                }
            }
            if(MAX_SCORE>0)
            {
                for(ii=0; ii<matched_pair_COUNT; ii++)
                {
                    in_index = matched_pair_IN[ii];
                    if(remove_i==in_index && ii!=MAX_INDEX)
                    {
                        matched_pair_SCORE[ii]=0;
//                      matched_pair_DB[ii]=-1;
//                      matched_pair_IN[ii]=-1;
                    }
                }
            }
        }
        //점수계산
        Total_NMS=0;
        STEP_MATCEDMINUTIAE_COUNT=0;
        for(j=0; j<matched_pair_COUNT; j++)
        {
            if(matched_pair_SCORE[j]<=0)continue;
            Total_NMS+=matched_pair_SCORE[j];
            m_StepMatchedMinutiaeDB[i][STEP_MATCEDMINUTIAE_COUNT] = matched_pair_DB[j];
            m_StepMatchedMinutiaeIN[i][STEP_MATCEDMINUTIAE_COUNT] = matched_pair_IN[j];
            m_StepMatchedMinutiaeScoreMinutiae[i][STEP_MATCEDMINUTIAE_COUNT] = matched_pair_SCORE[j];
            STEP_MATCEDMINUTIAE_COUNT++;
        }
        m_StepMatchedMinutiaeMatchedCount[i]=STEP_MATCEDMINUTIAE_COUNT;
        m_StepMatchedMinutiaeScore[i]=Total_NMS;

        if(Total_NMS >= D_DIRECT_MATCHING_SCORE)
        {
            MAX_INDEX = i;

#ifdef SAVE_MATCHING_INFO
            memset(&m_MatchingInfoDB, 0, sizeof(IBSM_MATCHING_INFO));
            memset(&m_MatchingInfoIN, 0, sizeof(IBSM_MATCHING_INFO));

            m_MatchingInfoDB.MatchedCount = m_StepMatchedMinutiaeMatchedCount[MAX_INDEX];
            m_MatchingInfoIN.MatchedCount = m_StepMatchedMinutiaeMatchedCount[MAX_INDEX];

            for(i=0; i<m_StepMatchedMinutiaeMatchedCount[MAX_INDEX]; i++)
            {
                Ref = m_StepMatchedMinutiaeDB[MAX_INDEX][i];
                Inp = m_StepMatchedMinutiaeIN[MAX_INDEX][i];

                m_MatchingInfoDB.MatchedMinutiae[i].x     = p_feavector_db->x[Ref];
                m_MatchingInfoDB.MatchedMinutiae[i].y     = p_feavector_db->y[Ref];
                m_MatchingInfoDB.MatchedMinutiae[i].angle = p_feavector_db->angle[Ref];
                m_MatchingInfoDB.MatchedMinutiae[i].type  = 0;
                m_MatchingInfoDB.MatchedScore[i]          = m_StepMatchedMinutiaeScoreMinutiae[MAX_INDEX][i];

                m_MatchingInfoIN.MatchedMinutiae[i].x     = p_feavector_in->x[Inp];
                m_MatchingInfoIN.MatchedMinutiae[i].y     = p_feavector_in->y[Inp];
                m_MatchingInfoIN.MatchedMinutiae[i].angle = p_feavector_in->angle[Inp];
                m_MatchingInfoIN.MatchedMinutiae[i].type  = 0;
                m_MatchingInfoIN.MatchedScore[i]          = m_StepMatchedMinutiaeScoreMinutiae[MAX_INDEX][i];
            }
#endif
            return Total_NMS;
        }
    }///////////////////////////////////////////MAIN FUNCTION END///////////////////////////////////

    //후보점 메칭한 것 중 Total_NMS가 가장 큰 것 찾기
    MAX_SCORE=0;
    for(i=0; i<Count_Candidate; i++)
    {
        SCORE = m_StepMatchedMinutiaeScore[i];
        if(MAX_SCORE<SCORE)
        {
            MAX_SCORE=SCORE;
            MAX_INDEX=i;
        }
    }

    if((MAX_SCORE<=0) || (m_StepMatchedMinutiaeMatchedCount[MAX_INDEX]<2))
    {
        return 0;
    }

    Total_NMS = m_StepMatchedMinutiaeScore[MAX_INDEX];

    /////////////////////////////////////////////////////////////////////////////
    //////글로벌 후보점에서 잘 맞은점 추가하기
    /////////////////////////////////////////////////////////////////////////////
    int matched_count=m_StepMatchedMinutiaeMatchedCount[MAX_INDEX];
    int MAX_INDEX2=0,MAX_SCORE2=0;
//  int SCORE_TRES=D_NEIGHBOR_PASS_SCORE;
    int Join_Tres=D_JOIN_ONENMS_LIM;

    //메칭된 미뉴샤 중 가장 점수 높은 미뉴샤 찾기
    for(i=0; i<matched_count; i++)
    {
        SCORE=m_StepMatchedMinutiaeScoreMinutiae[MAX_INDEX][i];
        if(SCORE>MAX_SCORE2)
        {
            MAX_SCORE2=SCORE;
            MAX_INDEX2=i;
        }
    }

    /////////////////////////////////////////////////////////////////////////////
    //////다른 트리에서는 메칭된 미뉴샤인데 메인트리에는 없는거....
    /////////////////////////////////////////////////////////////////////////////
    int JOIN_SCORE_TreeTRES=400,JOIN_SCORE_MinutiaeTRES=D_NEIGHBOR_PASS_SCORE;
    int matched_count_maxtree,matched_count_othertree;
    int other_Ref,other_Inp,other_Score;
    int m;

    for(i=0; i<Count_Candidate; i++)
    {
        SCORE=m_StepMatchedMinutiaeScore[i];
        if((i==MAX_INDEX) || (SCORE<JOIN_SCORE_TreeTRES))continue;

        //메인트리에서 메칭된 미뉴샤와 동일한 것인지를 판단한다.
        matched_count_othertree=m_StepMatchedMinutiaeMatchedCount[i];
        for(j=0; j<matched_count_othertree; j++)
        {
            SAVE=0;
            other_Ref = m_StepMatchedMinutiaeDB[i][j];
            other_Inp = m_StepMatchedMinutiaeIN[i][j];
            other_Score = m_StepMatchedMinutiaeScoreMinutiae[i][j];
            if(other_Score<JOIN_SCORE_MinutiaeTRES)continue;

            matched_count_maxtree=m_StepMatchedMinutiaeMatchedCount[MAX_INDEX];
            for(m=0; m<matched_count_maxtree; m++)
            {
                Ref = m_StepMatchedMinutiaeDB[MAX_INDEX][m];
                Inp = m_StepMatchedMinutiaeIN[MAX_INDEX][m];
                if(Ref == other_Ref || Inp == other_Inp)
                    SAVE=1;
            }
            //Global match를 이용해서 다른Tree의 미뉴샤를 Join한다.
            if(SAVE==0)
            {
                Ref = m_StepMatchedMinutiaeDB[MAX_INDEX][MAX_INDEX2];
                Inp = m_StepMatchedMinutiaeIN[MAX_INDEX][MAX_INDEX2];
                SCORE = _Judge_JointTree(Ref, other_Ref, Inp, other_Inp, p_feavector_db, p_feavector_in);
                if(SCORE>Join_Tres)
                {
                    Total_NMS = Total_NMS+other_Score;
                    //JOIN된 미뉴샤를 MAX_Tree 버퍼에 넣는다.
                    m_StepMatchedMinutiaeDB[MAX_INDEX][matched_count_maxtree] = other_Ref;
                    m_StepMatchedMinutiaeIN[MAX_INDEX][matched_count_maxtree] = other_Inp;
                    m_StepMatchedMinutiaeScoreMinutiae[MAX_INDEX][matched_count_maxtree] = other_Score;
                    m_StepMatchedMinutiaeMatchedCount[MAX_INDEX]+=1;
                    m_StepMatchedMinutiaeScore[MAX_INDEX]=Total_NMS;
                }
            }
        }
    }

#ifdef SAVE_MATCHING_INFO
    memset(&m_MatchingInfoDB, 0, sizeof(IBSM_MATCHING_INFO));
    memset(&m_MatchingInfoIN, 0, sizeof(IBSM_MATCHING_INFO));

    m_MatchingInfoDB.MatchedCount = m_StepMatchedMinutiaeMatchedCount[MAX_INDEX];
    m_MatchingInfoIN.MatchedCount = m_StepMatchedMinutiaeMatchedCount[MAX_INDEX];

    for(i=0; i<m_StepMatchedMinutiaeMatchedCount[MAX_INDEX]; i++)
    {
        Ref = m_StepMatchedMinutiaeDB[MAX_INDEX][i];
        Inp = m_StepMatchedMinutiaeIN[MAX_INDEX][i];

        m_MatchingInfoDB.MatchedMinutiae[i].x     = p_feavector_db->x[Ref];
        m_MatchingInfoDB.MatchedMinutiae[i].y     = p_feavector_db->y[Ref];
        m_MatchingInfoDB.MatchedMinutiae[i].angle = p_feavector_db->angle[Ref];
        m_MatchingInfoDB.MatchedMinutiae[i].type  = 0;
        m_MatchingInfoDB.MatchedScore[i]          = m_StepMatchedMinutiaeScoreMinutiae[MAX_INDEX][i];

        m_MatchingInfoIN.MatchedMinutiae[i].x     = p_feavector_in->x[Inp];
        m_MatchingInfoIN.MatchedMinutiae[i].y     = p_feavector_in->y[Inp];
        m_MatchingInfoIN.MatchedMinutiae[i].angle = p_feavector_in->angle[Inp];
        m_MatchingInfoIN.MatchedMinutiae[i].type  = 0;
        m_MatchingInfoIN.MatchedScore[i]          = m_StepMatchedMinutiaeScoreMinutiae[MAX_INDEX][i];
    }
#endif

#ifdef FVC_PENALTY
    Total_NMS = _PenaltyCalc(p_feavector_db,p_feavector_in,Total_NMS,m_StepMatchedMinutiaeDB[MAX_INDEX],
                            m_StepMatchedMinutiaeIN[MAX_INDEX],m_StepMatchedMinutiaeScoreMinutiae[MAX_INDEX],
                            m_StepMatchedMinutiaeMatchedCount[MAX_INDEX]);
    if(Total_NMS < 0)
    {
#ifdef SAVE_MATCHING_INFO
        memset(&m_MatchingInfoDB, 0, sizeof(IBSM_MATCHING_INFO));
        memset(&m_MatchingInfoIN, 0, sizeof(IBSM_MATCHING_INFO));
#endif
        Total_NMS = 0;
    }
#endif

    return Total_NMS;
}

int CIBSMAlgorithm::_GetAtanValue(int y, int x)
{
    if (y>=0)
    {
        if (x>=0)
        {
            return m_Atan2Table[y][x];
        }
        else
        {
            return 180 - m_Atan2Table[y][-x];
        }
    }
    else
    {
        if (x>=0)
        {
            return 360 - m_Atan2Table[-y][x];
        }
        else
        {
            return 180 + m_Atan2Table[-y][-x];
        }
    }
}

int CIBSMAlgorithm::_SearchCandidateMinutiae(FeatureVector* p_feavector_db,
                                            FeatureVector* p_feavector_in,
                                            CANDIDATE_Pair* p_Candidate_Pair)
{
    int Ref,Inp,Diff_Tetta;
    int Ref_Tetta,Ref_Num;
    int Inp_Tetta,Inp_Num;
    int minutiae_num_db,minutiae_num_in;
    int Score=0,MaxScore;
    unsigned char candidate_pair_db[MAX_MINUTIAE];
    unsigned char candidate_pair_in[MAX_MINUTIAE];
    short candidate_pair_score_db[MAX_MINUTIAE];
    short candidate_pair_score_in[MAX_MINUTIAE];

    int m_Candidate_Count,tmp;

    Ref_Num = p_feavector_db->num;
    Inp_Num = p_feavector_in->num;

#ifdef  __SCT_USING_FASTMATCHING__
    int CHECKScore=0,CHECKCount=0;
    int quality=p_feavector_db->quality,qualityIN=p_feavector_in->quality;
    CHECKCount = Ref_Num*60/100;

    #ifdef __SCT_HIGH_FASTMATCHING_DEVEL_MODE__
        if( m_MatchCoreLevel == 3 )
            CHECKCount = Ref_Num*60/100;
        else if( m_MatchCoreLevel == 2 )
            CHECKCount = Ref_Num*60/100;
        else if( m_MatchCoreLevel == 1 )
            CHECKCount = Ref_Num*60/100;
        else
            CHECKCount = Ref_Num*30/100;
    #endif
#endif

    memset(candidate_pair_db,0,sizeof(candidate_pair_db));
    memset(candidate_pair_in,0,sizeof(candidate_pair_in));
    memset(candidate_pair_score_db,0,sizeof(candidate_pair_score_db));
    memset(candidate_pair_score_in,0,sizeof(candidate_pair_score_in));
    m_Candidate_Count=0;

    for(Ref=0; Ref<Ref_Num; Ref++)
    {
#ifdef  __SCT_USING_FASTMATCHING__
        if( (quality>=1 || qualityIN>=1) && Ref>CHECKCount && CHECKScore<1000)
        {
            break;
        }
        else if((quality>=6 || qualityIN>=6) && Ref>CHECKCount && CHECKScore<2000)
        {
            break;
        }
        else if(CHECKScore>10000 || (quality>=6 && qualityIN>=6 && CHECKScore>4000))
        {
            break;
        }
#endif
        minutiae_num_db = Ref;
        if(!_DistanceCheck_ForCandidate(minutiae_num_db,p_feavector_db))
//      if( p_feavector_db->neighbor_info[minutiae_num_db].aver_dist < CUT_DISTANCE_FORCANDIDATE )
            continue;

        Ref_Tetta = p_feavector_db->angle[Ref];
        MaxScore=0;
        for(Inp=0; Inp<Inp_Num; Inp++)
        {
            minutiae_num_in = Inp;

            Inp_Tetta = p_feavector_in->angle[Inp];
            Diff_Tetta = Inp_Tetta-Ref_Tetta;
            Diff_Tetta = m_DiffAngleForMatching[Diff_Tetta+360];
            if(Diff_Tetta>D_ROTATION_LIM)
                continue;
/*
#ifdef  __SCT_USING_FASTMATCHING__
//          if( Diff_Tetta<40 && (abs(x-p_feavector_in->x[Inp])>200 || abs(y-p_feavector_in->y[Inp])>150) )             //버그 360도 일때 인증이 될까?
#ifdef __SCT_HIGH_FASTMATCHING_DEVEL_MODE__
            if( abs(p_feavector_db->x[Ref]-p_feavector_in->x[Inp]) > 176 || abs(p_feavector_db->y[Ref]-p_feavector_in->y[Inp]) > 144 )              //버그 360도 일때 인증이 될까?
#else
            if( abs(p_feavector_db->x[Ref]-p_feavector_in->x[Inp]) > 200 || abs(p_feavector_db->y[Ref]-p_feavector_in->y[Inp]) > 150 )              //버그 360도 일때 인증이 될까?
#endif
                continue;
#endif
*/
            if(candidate_pair_db[minutiae_num_db]>0 && candidate_pair_in[minutiae_num_in]>0)        //버그 360도 일때 인증이 될까?
                continue;

            if(!_DistanceCheck_ForCandidate(minutiae_num_in,p_feavector_in))
//          if( p_feavector_in->neighbor_info[minutiae_num_in].aver_dist < CUT_DISTANCE_FORCANDIDATE )
                continue;

            Score=_Neighbor_Matching_CalculationFORCANDIDATE(minutiae_num_db,minutiae_num_in,p_feavector_db,p_feavector_in);

            if(0>=Score)continue;

            if(candidate_pair_db[minutiae_num_db]==0 && candidate_pair_in[minutiae_num_in]==0)
            {
                candidate_pair_db[minutiae_num_db] = minutiae_num_in+1;
                candidate_pair_in[minutiae_num_in] = minutiae_num_db+1;
                candidate_pair_score_db[minutiae_num_db] = Score;
                candidate_pair_score_in[minutiae_num_in] = Score;
                MaxScore=Score;
            }
            else if((Score>MaxScore) && candidate_pair_db[minutiae_num_db]>0 && candidate_pair_in[minutiae_num_in]==0)
            {
                if( candidate_pair_score_db[minutiae_num_db] < Score )
                {
                    tmp=candidate_pair_db[minutiae_num_db]-1;
                    candidate_pair_in[tmp] = 0;
                    candidate_pair_score_in[tmp] = 0;

                    candidate_pair_db[minutiae_num_db] = minutiae_num_in+1;
                    candidate_pair_in[minutiae_num_in] = minutiae_num_db+1;
                    candidate_pair_score_db[minutiae_num_db] = Score;
                    candidate_pair_score_in[minutiae_num_in] = Score;
                    MaxScore=Score;
                }
            }
            else if((Score>MaxScore) && candidate_pair_db[minutiae_num_db]==0 && candidate_pair_in[minutiae_num_in]>0)
            {
                if( candidate_pair_score_in[minutiae_num_in] < Score )
                {
                    tmp=candidate_pair_in[minutiae_num_in]-1;
                    candidate_pair_db[tmp]=0;
                    candidate_pair_score_db[tmp] = 0;

                    candidate_pair_db[minutiae_num_db] = minutiae_num_in+1;
                    candidate_pair_in[minutiae_num_in] = minutiae_num_db+1;
                    candidate_pair_score_db[minutiae_num_db] = Score;
                    candidate_pair_score_in[minutiae_num_in] = Score;
                    MaxScore=Score;
                }
            }
        }
#ifdef  __SCT_USING_FASTMATCHING__
        CHECKScore+=MaxScore;
#endif
    }
#ifdef  __SCT_USING_FASTMATCHING__
#ifdef __SCT_HIGH_FASTMATCHING_DEVEL_MODE__
    if( CHECKScore < 500 )
#else
    if( CHECKScore < 300 )
#endif
    {
        return 0;
    }
#endif

    for(Ref=0; Ref<Ref_Num; Ref++)
    {
        if(candidate_pair_db[Ref]==0)continue;
        p_Candidate_Pair->Ref_Minutiae[m_Candidate_Count]=Ref;
        p_Candidate_Pair->Inp_Minutiae[m_Candidate_Count]=candidate_pair_db[Ref]-1;
        m_Candidate_Count++;
    }

    p_Candidate_Pair->Pair_Count=m_Candidate_Count;

    return m_Candidate_Count;
}

int CIBSMAlgorithm::_Neighbor_Matching_Calculation_New(int minutiae_num_db,int minutiae_num_in,FeatureVector* m_feavector_db,FeatureVector* m_feavector_in,
                                                     int *MATNEIGBOR8_DB, int *MATNEIGBOR8_IN, int *MATNEIGBOR8_COUNT)
{
    //////////////////////////////////////////////////////////////////////////////////////
    /////Penalty Calculation & Calculation of Neighbor Matching Score
    /////
    int i,in_negibor;
    int rIn;
    int rLim,angleLim,tettaLim;
#ifdef  MATCHING_USING_TABLE
    int RP_r,RP_angle,RP_tetta;
#else
    double RP_r,RP_angle,RP_tetta;
#endif
    int NMS;    //NMS=Neighbor Matching Score
    int diff_r,diff_angle,diff_thetta;
    int Total_NMS=0;
    int distIN,thettaIN,angleIN;
    int distDB,thettaDB,angleDB;
    int matched_pair_db[MAX_MINUTIAE];
    int matched_pair_in[MAX_MINUTIAE];
    int matched_pair_score[MAX_MINUTIAE];
    unsigned char matched_pair_count=0;
    int db_index,in_index,count,ii;
    int score,MAX_SCORE=0,MAX_INDEX=0;
    short overlap_minutiae_db[D_NEIGHBOR], overlap_minutiae_in[D_NEIGHBOR];

    memset(matched_pair_db,-1,sizeof(matched_pair_db));
    memset(matched_pair_in,-1,sizeof(matched_pair_in));
    memset(matched_pair_score,0,sizeof(matched_pair_score));

    for(in_negibor=0; in_negibor<D_NEIGHBOR; in_negibor++)
    {
        NMS=0;
        distIN=m_feavector_in->neighbor_info[minutiae_num_in].dist[in_negibor];
        thettaIN=m_feavector_in->neighbor_info[minutiae_num_in].thetta[in_negibor];
        angleIN=m_feavector_in->neighbor_info[minutiae_num_in].angle[in_negibor];

        for(i=0; i<D_NEIGHBOR; i++)
        {
            distDB=m_feavector_db->neighbor_info[minutiae_num_db].dist[i];
            thettaDB=m_feavector_db->neighbor_info[minutiae_num_db].thetta[i];
            angleDB=m_feavector_db->neighbor_info[minutiae_num_db].angle[i];

            rIn = distDB;
            diff_angle = angleDB - angleIN;
            diff_angle = m_DiffAngleForMatching[diff_angle+360];

#ifdef  MATCHING_USING_TABLE
            angleLim = m_AngleLimForMatching[rIn];
#else
            rLim = (int)(rMin + rIn*rInc);
            angleLim = (int)(angleMin + rLim*(180./pi/(float)rIn));
            if(angleLim>=angleMax)
                angleLim=angleMax;
#endif

            if(diff_angle<=angleLim)
            {
                diff_thetta = thettaDB - thettaIN;
                diff_thetta = m_DiffAngleForMatching[diff_thetta+360];

                tettaLim=TETTA_LIM;
                if(diff_thetta<=tettaLim)
                {
                    diff_r = abs(distDB - distIN);
                    rLim = m_RLimForMatching[rIn];
                    if(diff_r<=rLim)
                    {
#ifdef  MATCHING_USING_TABLE
                        RP_r =m_RpRForMatching[rIn];
                        RP_angle = m_RpAngleForMatching[rIn];
#else
                        RP_r = P_rMax/rLim;
                        RP_angle = P_rMax/angleLim;
#endif
                        RP_tetta = RP_THETTA;

#ifdef  MATCHING_USING_TABLE
    #ifdef WAS_186
                        NMS = (NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + (rIn<<10) / 186 ) >> 10;//+ (rIn<<9))>>10;
    #else
                        NMS = (NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + (rIn<<9) )>>10;
    #endif
#else
    #ifdef WAS_186
                        NMS = (int)(NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + rIn/186.);
    #else
                        NMS = (int)(NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + rIn*0.5);
    #endif
#endif
                        matched_pair_db[matched_pair_count] = i;
                        matched_pair_in[matched_pair_count] = in_negibor;
                        matched_pair_score[matched_pair_count] = NMS;
                        matched_pair_count++;
                    }
                }
            }
        }
    }

    //중복 미뉴샤 제거
    MAX_SCORE=0;
    memset(overlap_minutiae_db,0,sizeof(overlap_minutiae_db));
    memset(overlap_minutiae_in,0,sizeof(overlap_minutiae_in));

    for(i=0; i<matched_pair_count; i++)
    {
        db_index = matched_pair_db[i];
        overlap_minutiae_db[db_index] += 1;
        in_index = matched_pair_in[i];
        overlap_minutiae_in[in_index] += 1;
    }

    //DB네이버 중복 삭제
    for(i=0; i<D_NEIGHBOR; i++)
    {
        count = overlap_minutiae_db[i];
        if(count<2) continue;

        MAX_SCORE=0;
        for(ii=0; ii<matched_pair_count; ii++)
        {
            db_index = matched_pair_db[ii];
            if(i==db_index)
            {
                score =matched_pair_score[ii];
                if(score>MAX_SCORE)
                {
                    MAX_SCORE=score;
                    MAX_INDEX=ii;
                }
            }
        }
        for(ii=0; ii<matched_pair_count; ii++)
        {
            db_index = matched_pair_db[ii];
            if(i==db_index && ii!=MAX_INDEX)
            {
                matched_pair_score[ii]=0;
                matched_pair_db[ii]=-1;
                matched_pair_in[ii]=-1;
            }
        }
    }
    //INPUT네이버 중복 삭제
    for(i=0; i<D_NEIGHBOR; i++)
    {
        count = overlap_minutiae_in[i];
        if(count<2) continue;

        MAX_SCORE=0;
        for(ii=0; ii<matched_pair_count; ii++)
        {
            in_index = matched_pair_in[ii];
            if(i==in_index)
            {
                score =matched_pair_score[ii];
                if(score>MAX_SCORE)
                {
                    MAX_SCORE=score;
                    MAX_INDEX=ii;
                }
            }
        }
        if(MAX_SCORE>0)
        {
            for(ii=0; ii<matched_pair_count; ii++)
            {
                in_index = matched_pair_in[ii];
                if(i==in_index && ii!=MAX_INDEX)
                {
                    matched_pair_score[ii]=0;
                    matched_pair_db[ii]=-1;
                    matched_pair_in[ii]=-1;
                }
            }
        }
    }

    //초기화
    for(i=0; i<D_NEIGHBOR; i++)
    {
        MATNEIGBOR8_DB[i]=-1;
        MATNEIGBOR8_IN[i]=-1;
    }

    count=0;
    // bug fixed
    for(ii=0; ii<matched_pair_count; ii++)      // for(ii=0; ii<D_NEIGHBOR; ii++)
    {
        db_index = matched_pair_db[ii];
        in_index = matched_pair_in[ii];
        if(db_index<0 || in_index<0)continue;

        MATNEIGBOR8_DB[count] = m_feavector_db->neighbor_info[minutiae_num_db].minutiae_num[db_index];
        MATNEIGBOR8_IN[count] = m_feavector_in->neighbor_info[minutiae_num_in].minutiae_num[in_index];
        count++;
    }
    *MATNEIGBOR8_COUNT=count;

    count=0;
    Total_NMS=0;
    for(i=0; i<matched_pair_count; i++)
    {
        if(matched_pair_score[i]<=0)continue;

        Total_NMS+=matched_pair_score[i];
    }

    if(Total_NMS>=D_NEIGHBOR_PASS_SCORE)
        return Total_NMS;

    return 0;
}

int CIBSMAlgorithm::_Judge_TracingTree(int origin_db, int target_db, int origin_in, int target_in,
									 FeatureVector* m_feavector_db,FeatureVector* m_feavector_in, 
									 int* Diff_angle, int* Diff_thetta)
{
    int x,y;
    int dist_db,dist_in,angle_db,angle_in;
    int tetta,angle;
    int delta_row,delta_col;
    int delta_row2,delta_col2;
    int rIn;
    int rLim,angleLim,tettaLim;
#ifdef  MATCHING_USING_TABLE
    int RP_r,RP_angle,RP_tetta;
#else
    double RP_r,RP_angle,RP_tetta;
#endif
    int diff_r,diff_angle,diff_thetta,NMS;

    delta_row = m_feavector_db->y[target_db]-m_feavector_db->y[origin_db];
    delta_col = m_feavector_db->x[target_db]-m_feavector_db->x[origin_db];
    delta_row2 = m_feavector_in->y[target_in]-m_feavector_in->y[origin_in];
    delta_col2 = m_feavector_in->x[target_in]-m_feavector_in->x[origin_in];
    //Distance 구하기
    x = abs(delta_col);
    y = abs(delta_row);
    dist_db=m_SqrtTable[y][x];
//  dist_db=(int)sqrt((double)y*y+x*x);
    if(dist_db<=0)return 0;
    x = abs(delta_col2);
    y = abs(delta_row2);
    dist_in=m_SqrtTable[y][x];
//  dist_in=(int)sqrt((double)y*y+x*x);
    if(dist_in<=0)return 0;

    rIn = dist_db;

    /////////////////////RLIM_FORMATCHING/////////////////
#ifdef  MATCHING_USING_TABLE
    rLim = m_RLimForMatching[rIn];
#else
    rLim = (int)(rMin + rInc*rIn);
#endif
    //////////////////////////////////////////////////////
    diff_r = abs(dist_db - dist_in);
    if(diff_r>rLim)return 0;

    //angle 구하기
    tetta = m_feavector_db->angle[origin_db];
    angle = _GetAtanValue(delta_row, delta_col) - tetta;
    if (angle<0)
        angle += 360;
    angle_db=angle;

    tetta = m_feavector_in->angle[origin_in];
    angle = _GetAtanValue(delta_row2, delta_col2) - tetta;
    if (angle<0)
        angle += 360;
    angle_in=angle;

    //////////////ANGLELIM_FORMATCHING///////////////
#ifdef  MATCHING_USING_TABLE
    angleLim = m_AngleLimForMatching[rIn];
#else
    rLim = (int)(rMin + rInc*rIn);
    angleLim = (int)(angleMin + rLim*(180./pi/(float)rIn));
    if(angleLim>=angleMax)
        angleLim=angleMax;
#endif
    /////////////////////////////////////////////////
    diff_angle = angle_db - angle_in;
    diff_angle = m_DiffAngleForMatching[diff_angle+360];
    if(diff_angle>angleLim)return 0;

    //tetta 구하기
    int tetta1,tetta_db,tetta_in;

    tetta = m_feavector_db->angle[origin_db];
    tetta1= m_feavector_db->angle[target_db];
    tetta_db = tetta1-tetta;
    if(tetta_db<0)
        tetta_db += 360;
    tetta = m_feavector_in->angle[origin_in];
    tetta1= m_feavector_in->angle[target_in];
    tetta_in = tetta1-tetta;
    if(tetta_in<0)
        tetta_in += 360;

    tettaLim=TETTA_LIM;
    diff_thetta = tetta_db - tetta_in;
    diff_thetta=m_DiffAngleForMatching[diff_thetta+360];
    if(diff_thetta>tettaLim)return 0;

    /////////////////////////////////////////////////////////
#ifdef  MATCHING_USING_TABLE
    RP_r = m_RpRForMatching[rIn];
    RP_angle = m_RpAngleForMatching[rIn];
#else
    RP_r = P_rMax/rLim;
    RP_angle = P_rMax/angleLim;
#endif
    RP_tetta =RP_THETTA;
    /////////////////////////////////////////////////////////

    *Diff_angle = diff_angle;
    *Diff_thetta = diff_thetta;

#ifdef  MATCHING_USING_TABLE
    #ifdef WAS_186
    NMS = (NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + (rIn<<10)/186)>>10;//+ (rIn<<9))>>10;
    #else
    NMS = (NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + (rIn<<10)/D_WAS)>>10;        //버그 WAS 조정해라!!!
    #endif
#else
    #ifdef WAS_186
    NMS = (int)(NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + rIn/186.);
    #else
    NMS = (int)(NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + rIn/186.);
    #endif
#endif

    if(NMS<D_JOIN_TRACENMS_LIM)
    {
        *Diff_angle=0;
        return 0;
    }
    return NMS;
}

int CIBSMAlgorithm::_Judge_JointTree(int origin_db, int target_db, int origin_in, int target_in,FeatureVector* p_feavector_db,FeatureVector* p_feavector_in)
{
    int x,y;
    int dist_db,dist_in,angle_db,angle_in;
    int tetta,angle;
    int delta_row,delta_col;
    int delta_row2,delta_col2;
    int rIn;
    int rLim,angleLim,tettaLim;
#ifdef  MATCHING_USING_TABLE
    int RP_r,RP_angle,RP_tetta;
#else
    double RP_r,RP_angle,RP_tetta;
#endif
    int diff_r,diff_angle,diff_thetta,NMS;

    delta_row = p_feavector_db->y[target_db]-p_feavector_db->y[origin_db];
    delta_col = p_feavector_db->x[target_db]-p_feavector_db->x[origin_db];
    delta_row2 = p_feavector_in->y[target_in]-p_feavector_in->y[origin_in];
    delta_col2 = p_feavector_in->x[target_in]-p_feavector_in->x[origin_in];
    //Distance 구하기
    x = abs(delta_col);
    y = abs(delta_row);
    dist_db=m_SqrtTable[y][x];
//  dist_db=(int)sqrt((double)y*y+x*x);
    x = abs(delta_col2);
    y = abs(delta_row2);
    dist_in=m_SqrtTable[y][x];
//  dist_in=(int)sqrt((double)y*y+x*x);

    rIn = dist_db;
    rLim = m_RLimForMatching[rIn];
    diff_r = abs(dist_db - dist_in);
    if(diff_r>rLim)return 0;

    //angle 구하기
    tetta = p_feavector_db->angle[origin_db];
    angle = _GetAtanValue(delta_row, delta_col) - tetta;
    if (angle<0)
        angle += 360;
    angle_db=angle;
    tetta = p_feavector_in->angle[origin_in];

    angle = _GetAtanValue(delta_row2, delta_col2) - tetta;
    if (angle<0)
        angle += 360;
    angle_in=angle;

    diff_angle = angle_db - angle_in;
    diff_angle=m_DiffAngleForMatching[diff_angle+360];
    angleLim = m_AngleLimForMatching[rIn];
    if(diff_angle>angleLim)return 0;


    //tetta 구하기
    int tetta1,tetta_db,tetta_in;

    tetta = p_feavector_db->angle[origin_db];
    tetta1= p_feavector_db->angle[target_db];
    tetta_db = tetta1-tetta;
    if(tetta_db<0)
        tetta_db += 360;
    tetta = p_feavector_in->angle[origin_in];
    tetta1= p_feavector_in->angle[target_in];
    tetta_in = tetta1-tetta;
    if(tetta_in<0)
        tetta_in += 360;

    tettaLim = TETTA_LIM;
    diff_thetta = tetta_db - tetta_in;
    diff_thetta=m_DiffAngleForMatching[diff_thetta+360];
    if(diff_thetta>tettaLim)return 0;

    ////////////////////////매칭해보기
    /////////////////////////RP_R_FORMATCHING////////////////
#ifdef  MATCHING_USING_TABLE
    RP_r =m_RpRForMatching[rIn];
#else
    RP_r = P_rMax/rLim;
#endif
    /////////////////////////////////////////////////////////

    /////////////////////////RP_ANGLE_FORMATCHING////////////////
#ifdef  MATCHING_USING_TABLE
    RP_angle = m_RpAngleForMatching[rIn];
#else
    RP_angle = P_rMax/angleLim;
#endif
    /////////////////////////////////////////////////////////
    RP_tetta =RP_THETTA;

#ifdef  MATCHING_USING_TABLE
    #ifdef WAS_186
    NMS = (NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + (rIn<<10)/186)>>10;//+ (rIn<<9))>>10;
    #else
    NMS = (NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + (rIn<<10)/D_WAS)>>10;        //버그 WAS 조정해라!!!
    #endif
#else
    #ifdef WAS_186
    NMS = (int)(NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + rIn/186.);
    #else
    NMS = (int)(NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + rIn/186.);
    #endif
#endif

    if(NMS<D_JOIN_ONENMS_LIM)
        return 0;
    return NMS;
}

int CIBSMAlgorithm::_DistanceCheck_ForCandidate(int minutiae_num, FeatureVector* feavector)
{
    int i,distance=0;
    int aver_dist;

    for( i=0; i<D_NEIGHBOR; i++ )
    {
        distance+=feavector->neighbor_info[minutiae_num].dist[i];
    }

    aver_dist = distance>>3;

    if( aver_dist < CUT_DISTANCE_FORCANDIDATE )
        return 0;

    return 1;
}

#ifdef __SCT_HIGH_FASTMATCHING_DEVEL_MODE__

int CIBSMAlgorithm::_Neighbor_Matching_CalculationFORCANDIDATE(int minutiae_num_db,int minutiae_num_in,FeatureVector* p_feavector_db,FeatureVector* p_feavector_in)
{
    //////////////////////////////////////////////////////////////////////////////////////
    /////Penalty Calculation & Calculation of Neighbor Matching Score
    /////
    int in_negibor;//, i;
    int rIn;
    int rLim,angleLim,tettaLim;
#ifdef  MATCHING_USING_TABLE
    int RP_r,RP_angle,RP_tetta;
#else
    double RP_r,RP_angle,RP_tetta;
#endif
    int NMS;    //NMS=Neighbor Matching Score
    int diff_r,diff_angle,diff_thetta;
//  unsigned char AdhearingCheck[D_NEIGHBOR][D_NEIGHBOR];
//  unsigned char AdhearingCheck_DB[D_NEIGHBOR]={0};
//  unsigned char AdhearingCheck_IN[D_NEIGHBOR]={0};
    int DIST_PENALTY=0,DIST_PENALTY_COUNT=0;
    int Total_NMS,maxNMS;
    int distIN,thettaIN,angleIN;


//  memset(AdhearingCheck,0,D_NEIGHBOR*D_NEIGHBOR);
//  memset(AdhearingCheck_DB,0,D_NEIGHBOR);
//  memset(AdhearingCheck_IN,0,D_NEIGHBOR);
    Total_NMS=0;


#if (__SCT_PRODUCT_TYPE__ == __SCT_PRODUCT_MODULE__ || __SCT_PRODUCT_TYPE__ == __SCT_PRODUCT_TnA__)
    for( in_negibor=0; in_negibor<D_NEIGHBOR; in_negibor++ )
    {
        for( i=0; i<D_NEIGHBOR; i++ )
        {
            distIN=p_feavector_in->neighbor_info[minutiae_num_in].dist[in_negibor];
            thettaIN=p_feavector_in->neighbor_info[minutiae_num_in].thetta[in_negibor];
            angleIN=p_feavector_in->neighbor_info[minutiae_num_in].angle[in_negibor];

            rIn = p_feavector_db->neighbor_info[minutiae_num_db].dist[i];
            diff_angle = p_feavector_db->neighbor_info[minutiae_num_db].angle[i] - angleIN;
            diff_angle=DIFFANGLE_FORMATCHING[diff_angle+360];
            angleLim = ANGLELIM_FORMATCHING[rIn];
            if(diff_angle<=angleLim)
            {
                diff_thetta = p_feavector_db->neighbor_info[minutiae_num_db].thetta[i] - thettaIN;
                diff_thetta=DIFFANGLE_FORMATCHING[diff_thetta+360];
                tettaLim = TETTA_LIM;
                if(diff_thetta<=tettaLim)
                {
                    diff_r = abs(p_feavector_db->neighbor_info[minutiae_num_db].dist[i] - distIN);
                    rLim = RLIM_FORMATCHING[rIn];
                    if(diff_r<=rLim)
                    {
                        RP_r = RP_R_FORMATCHING[rIn];
                        RP_angle = RP_ANGLE_FORMATCHING[rIn];
                        RP_tetta = RP_THETTA;
#ifdef WAS_186
                        NMS = (NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + (rIn<<10)/186)>>10;//+ (rIn<<9))>>10;
#else
                        NMS = (NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + (rIn<<9))>>10;
#endif
                        DIST_PENALTY = p_feavector_db->neighbor_info[minutiae_num_db].dist[i];  //원본이 이건데 버그 같다.
//                      DIST_PENALTY += m_feavector_db->neighbor_info[minutiae_num_db].dist[0];
                        DIST_PENALTY_COUNT++;
                        AdhearingCheck[in_negibor][i] = NMS;
                        AdhearingCheck_DB[i]++;
                        AdhearingCheck_IN[in_negibor]++;
                        Total_NMS += NMS;
                    }
                }
            }
        }
    }
#else
    for(in_negibor=0; in_negibor<D_NEIGHBOR; in_negibor++)
    {
        maxNMS = 0;
        distIN=p_feavector_in->neighbor_info[minutiae_num_in].dist[in_negibor];
        thettaIN=p_feavector_in->neighbor_info[minutiae_num_in].thetta[in_negibor];
        angleIN=p_feavector_in->neighbor_info[minutiae_num_in].angle[in_negibor];

        rIn = p_feavector_db->neighbor_info[minutiae_num_db].dist[0];
        diff_angle = p_feavector_db->neighbor_info[minutiae_num_db].angle[0] - angleIN;
        diff_angle=m_DiffAngleForMatching[diff_angle+360];
        angleLim = m_AngleLimForMatching[rIn];
        if(diff_angle<=angleLim)
        {
            diff_thetta = p_feavector_db->neighbor_info[minutiae_num_db].thetta[0] - thettaIN;
            diff_thetta=m_DiffAngleForMatching[diff_thetta+360];
            tettaLim = TETTA_LIM;
            if(diff_thetta<=tettaLim)
            {
                diff_r = abs(p_feavector_db->neighbor_info[minutiae_num_db].dist[0] - distIN);
                rLim = m_RLimForMatching[rIn];
                if(diff_r<=rLim)
                {
                    /////////////////////////RP_R_FORMATCHING////////////////
#ifdef  MATCHING_USING_TABLE
                    RP_r =m_RpRForMatching[rIn];
                    RP_angle = m_RpAngleForMatching[rIn];
                    RP_tetta = RP_THETTA;
#else
                    RP_r = P_rMax/rLim;
                    RP_angle = P_rMax/angleLim;
                    RP_tetta = RP_THETTA;
#endif

#ifdef MATCHING_USING_TABLE
    #ifdef WAS_186
                    NMS = (NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + (rIn<<10)/186)>>10;//+ (rIn<<9))>>10;
    #else
                    NMS = (NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + (rIn<<9))>>10;
    #endif
#else
    #ifdef WAS_186
                    NMS = (int)(NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + rIn/186.);
    #else
                    NMS = (int)(NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + rIn*0.5);
    #endif
#endif
                    DIST_PENALTY = p_feavector_db->neighbor_info[minutiae_num_db].dist[0];  //원본이 이건데 버그 같다.
//                  DIST_PENALTY += m_feavector_db->neighbor_info[minutiae_num_db].dist[0];
                    DIST_PENALTY_COUNT++;
//                  AdhearingCheck[in_negibor][0] = NMS;
//                  AdhearingCheck_DB[0]++;
//                  AdhearingCheck_IN[in_negibor]++;
                    if( NMS > maxNMS )
                    {
                        Total_NMS += (NMS-maxNMS);
                        maxNMS = NMS;
                    }
                }
            }
        }

        rIn = p_feavector_db->neighbor_info[minutiae_num_db].dist[1];
        diff_angle = p_feavector_db->neighbor_info[minutiae_num_db].angle[1] - angleIN;
        diff_angle=m_DiffAngleForMatching[diff_angle+360];
        angleLim = m_AngleLimForMatching[rIn];
        if(diff_angle<=angleLim)
        {
            diff_thetta = p_feavector_db->neighbor_info[minutiae_num_db].thetta[1] - thettaIN;
            diff_thetta=m_DiffAngleForMatching[diff_thetta+360];
            tettaLim = TETTA_LIM;
            if(diff_thetta<=tettaLim)
            {
                diff_r = abs(p_feavector_db->neighbor_info[minutiae_num_db].dist[1] - distIN);
                rLim = m_RLimForMatching[rIn];
                if(diff_r<=rLim)
                {
#ifdef  MATCHING_USING_TABLE
                    RP_r =m_RpRForMatching[rIn];
                    RP_angle = m_RpAngleForMatching[rIn];
                    RP_tetta =RP_THETTA;
#else
                    RP_r = P_rMax/rLim;
                    RP_angle = P_rMax/angleLim;
                    RP_tetta = RP_THETTA;
#endif

#ifdef MATCHING_USING_TABLE
    #ifdef WAS_186
                    NMS = (NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + (rIn<<10)/186)>>10;//+ (rIn<<9))>>10;
    #else
                    NMS = (NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + (rIn<<9))>>10;
    #endif
#else
    #ifdef WAS_186
                    NMS = (int)(NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + rIn/186.);
    #else
                    NMS = (int)(NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + rIn*0.5);
    #endif
#endif
                    DIST_PENALTY = p_feavector_db->neighbor_info[minutiae_num_db].dist[1];  //원본이 이건데 버그 같다.
//                  DIST_PENALTY += m_feavector_db->neighbor_info[minutiae_num_db].dist[1];
                    DIST_PENALTY_COUNT++;
//                  AdhearingCheck[in_negibor][1] = NMS;
//                  AdhearingCheck_DB[1]++;
//                  AdhearingCheck_IN[in_negibor]++;
                    if( NMS > maxNMS )
                    {
                        Total_NMS += (NMS-maxNMS);
                        maxNMS = NMS;
                    }
                }
            }
        }

        rIn = p_feavector_db->neighbor_info[minutiae_num_db].dist[2];
        diff_angle = p_feavector_db->neighbor_info[minutiae_num_db].angle[2] - angleIN;
        diff_angle=m_DiffAngleForMatching[diff_angle+360];
        angleLim = m_AngleLimForMatching[rIn];
        if(diff_angle<=angleLim)
        {
            diff_thetta = p_feavector_db->neighbor_info[minutiae_num_db].thetta[2] - thettaIN;
            diff_thetta=m_DiffAngleForMatching[diff_thetta+360];
            tettaLim = TETTA_LIM;
            if(diff_thetta<=tettaLim)
            {
                diff_r = abs(p_feavector_db->neighbor_info[minutiae_num_db].dist[2] - distIN);
                rLim = m_RLimForMatching[rIn];
                if(diff_r<=rLim)
                {
#ifdef  MATCHING_USING_TABLE
                    RP_r =m_RpRForMatching[rIn];
                    RP_angle = m_RpAngleForMatching[rIn];
                    RP_tetta =RP_THETTA;
#else
                    RP_r = P_rMax/rLim;
                    RP_angle = P_rMax/angleLim;
                    RP_tetta = RP_THETTA;
#endif

#ifdef MATCHING_USING_TABLE
    #ifdef WAS_186
                    NMS = (NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + (rIn<<10)/186)>>10;//+ (rIn<<9))>>10;
    #else
                    NMS = (NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + (rIn<<9))>>10;
    #endif
#else
    #ifdef WAS_186
                    NMS = (int)(NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + rIn/186.);
    #else
                    NMS = (int)(NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + rIn*0.5);
    #endif
#endif
                    DIST_PENALTY = p_feavector_db->neighbor_info[minutiae_num_db].dist[2];  //원본이 이건데 버그 같다.
//                  DIST_PENALTY += m_feavector_db->neighbor_info[minutiae_num_db].dist[2];
                    DIST_PENALTY_COUNT++;
//                  AdhearingCheck[in_negibor][2] = NMS;
//                  AdhearingCheck_DB[2]++;
//                  AdhearingCheck_IN[in_negibor]++;
                    if( NMS > maxNMS )
                    {
                        Total_NMS += (NMS-maxNMS);
                        maxNMS = NMS;
                    }
                }
            }
        }

        rIn = p_feavector_db->neighbor_info[minutiae_num_db].dist[3];
        diff_angle = p_feavector_db->neighbor_info[minutiae_num_db].angle[3] - angleIN;
        diff_angle=m_DiffAngleForMatching[diff_angle+360];
        angleLim = m_AngleLimForMatching[rIn];
        if(diff_angle<=angleLim)
        {
            diff_thetta = p_feavector_db->neighbor_info[minutiae_num_db].thetta[3] - thettaIN;
            diff_thetta=m_DiffAngleForMatching[diff_thetta+360];
            tettaLim = TETTA_LIM;
            if(diff_thetta<=tettaLim)
            {
                diff_r = abs(p_feavector_db->neighbor_info[minutiae_num_db].dist[3] - distIN);
                rLim = m_RLimForMatching[rIn];
                if(diff_r<=rLim)
                {
#ifdef  MATCHING_USING_TABLE
                    RP_r =m_RpRForMatching[rIn];
                    RP_angle = m_RpAngleForMatching[rIn];
                    RP_tetta =RP_THETTA;
#else
                    RP_r = P_rMax/rLim;
                    RP_angle = P_rMax/angleLim;
                    RP_tetta = RP_THETTA;
#endif

#ifdef MATCHING_USING_TABLE
    #ifdef WAS_186
                    NMS = (NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + (rIn<<10)/186)>>10;//+ (rIn<<9))>>10;
    #else
                    NMS = (NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + (rIn<<9))>>10;
    #endif
#else
    #ifdef WAS_186
                    NMS = (int)(NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + rIn/186.);
    #else
                    NMS = (int)(NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + rIn*0.5);
    #endif
#endif
                    DIST_PENALTY = p_feavector_db->neighbor_info[minutiae_num_db].dist[3];  //원본이 이건데 버그 같다.
//                  DIST_PENALTY += m_feavector_db->neighbor_info[minutiae_num_db].dist[3];
                    DIST_PENALTY_COUNT++;
//                  AdhearingCheck[in_negibor][3] = NMS;
//                  AdhearingCheck_DB[3]++;
//                  AdhearingCheck_IN[in_negibor]++;
                    if( NMS > maxNMS )
                    {
                        Total_NMS += (NMS-maxNMS);
                        maxNMS = NMS;
                    }
                }
            }
        }

        rIn = p_feavector_db->neighbor_info[minutiae_num_db].dist[4];
        diff_angle = p_feavector_db->neighbor_info[minutiae_num_db].angle[4] - angleIN;
        diff_angle=m_DiffAngleForMatching[diff_angle+360];
        angleLim = m_AngleLimForMatching[rIn];
        if(diff_angle<=angleLim)
        {
            diff_thetta = p_feavector_db->neighbor_info[minutiae_num_db].thetta[4] - thettaIN;
            diff_thetta=m_DiffAngleForMatching[diff_thetta+360];
            tettaLim = TETTA_LIM;
            if(diff_thetta<=tettaLim)
            {
                diff_r = abs(p_feavector_db->neighbor_info[minutiae_num_db].dist[4] - distIN);
                rLim = m_RLimForMatching[rIn];
                if(diff_r<=rLim)
                {
#ifdef  MATCHING_USING_TABLE
                    RP_r =m_RpRForMatching[rIn];
                    RP_angle = m_RpAngleForMatching[rIn];
                    RP_tetta =RP_THETTA;
#else
                    RP_r = P_rMax/rLim;
                    RP_angle = P_rMax/angleLim;
                    RP_tetta = RP_THETTA;
#endif

#ifdef MATCHING_USING_TABLE
    #ifdef WAS_186
                    NMS = (NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + (rIn<<10)/186)>>10;//+ (rIn<<9))>>10;
    #else
                    NMS = (NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + (rIn<<9))>>10;
    #endif
#else
    #ifdef WAS_186
                    NMS = (int)(NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + rIn/186.);
    #else
                    NMS = (int)(NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + rIn*0.5);
    #endif
#endif
                    DIST_PENALTY = p_feavector_db->neighbor_info[minutiae_num_db].dist[4];  //원본이 이건데 버그 같다.
//                  DIST_PENALTY += m_feavector_db->neighbor_info[minutiae_num_db].dist[4];
                    DIST_PENALTY_COUNT++;
//                  AdhearingCheck[in_negibor][4] = NMS;
//                  AdhearingCheck_DB[4]++;
//                  AdhearingCheck_IN[in_negibor]++;
                    if( NMS > maxNMS )
                    {
                        Total_NMS += (NMS-maxNMS);
                        maxNMS = NMS;
                    }
                }
            }
        }

        rIn = p_feavector_db->neighbor_info[minutiae_num_db].dist[5];
        diff_angle = p_feavector_db->neighbor_info[minutiae_num_db].angle[5] - angleIN;
        diff_angle=m_DiffAngleForMatching[diff_angle+360];
        angleLim = m_AngleLimForMatching[rIn];
        if(diff_angle<=angleLim)
        {
            diff_thetta = p_feavector_db->neighbor_info[minutiae_num_db].thetta[5] - thettaIN;
            diff_thetta=m_DiffAngleForMatching[diff_thetta+360];
            tettaLim = TETTA_LIM;
            if(diff_thetta<=tettaLim)
            {
                diff_r = abs(p_feavector_db->neighbor_info[minutiae_num_db].dist[5] - distIN);
                rLim = m_RLimForMatching[rIn];
                if(diff_r<=rLim)
                {
#ifdef  MATCHING_USING_TABLE
                    RP_r =m_RpRForMatching[rIn];
                    RP_angle = m_RpAngleForMatching[rIn];
                    RP_tetta =RP_THETTA;
#else
                    RP_r = P_rMax/rLim;
                    RP_angle = P_rMax/angleLim;
                    RP_tetta = RP_THETTA;
#endif

#ifdef MATCHING_USING_TABLE
    #ifdef WAS_186
                    NMS = (NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + (rIn<<10)/186)>>10;//+ (rIn<<9))>>10;
    #else
                    NMS = (NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + (rIn<<9))>>10;
    #endif
#else
    #ifdef WAS_186
                    NMS = (int)(NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + rIn/186.);
    #else
                    NMS = (int)(NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + rIn*0.5);
    #endif
#endif
                    DIST_PENALTY = p_feavector_db->neighbor_info[minutiae_num_db].dist[5];  //원본이 이건데 버그 같다.
//                  DIST_PENALTY += m_feavector_db->neighbor_info[minutiae_num_db].dist[5];
                    DIST_PENALTY_COUNT++;
//                  AdhearingCheck[in_negibor][5] = NMS;
//                  AdhearingCheck_DB[5]++;
//                  AdhearingCheck_IN[in_negibor]++;
                    if( NMS > maxNMS )
                    {
                        Total_NMS += (NMS-maxNMS);
                        maxNMS = NMS;
                    }
                }
            }
        }

        rIn = p_feavector_db->neighbor_info[minutiae_num_db].dist[6];
        diff_angle = p_feavector_db->neighbor_info[minutiae_num_db].angle[6] - angleIN;
        diff_angle=m_DiffAngleForMatching[diff_angle+360];
        angleLim = m_AngleLimForMatching[rIn];
        if(diff_angle<=angleLim)
        {
            diff_thetta = p_feavector_db->neighbor_info[minutiae_num_db].thetta[6] - thettaIN;
            diff_thetta=m_DiffAngleForMatching[diff_thetta+360];
            tettaLim = TETTA_LIM;
            if(diff_thetta<=tettaLim)
            {
                diff_r = abs(p_feavector_db->neighbor_info[minutiae_num_db].dist[6] - distIN);
                rLim = m_RLimForMatching[rIn];
                if(diff_r<=rLim)
                {
#ifdef  MATCHING_USING_TABLE
                    RP_r =m_RpRForMatching[rIn];
                    RP_angle = m_RpAngleForMatching[rIn];
                    RP_tetta =RP_THETTA;
#else
                    RP_r = P_rMax/rLim;
                    RP_angle = P_rMax/angleLim;
                    RP_tetta = RP_THETTA;
#endif

#ifdef MATCHING_USING_TABLE
    #ifdef WAS_186
                    NMS = (NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + (rIn<<10)/186)>>10;//+ (rIn<<9))>>10;
    #else
                    NMS = (NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + (rIn<<9))>>10;
    #endif
#else
    #ifdef WAS_186
                    NMS = (int)(NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + rIn/186.);
    #else
                    NMS = (int)(NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + rIn*0.5);
    #endif
#endif
                    DIST_PENALTY = p_feavector_db->neighbor_info[minutiae_num_db].dist[6];  //원본이 이건데 버그 같다.
//                  DIST_PENALTY += m_feavector_db->neighbor_info[minutiae_num_db].dist[6];
                    DIST_PENALTY_COUNT++;
//                  AdhearingCheck[in_negibor][6] = NMS;
//                  AdhearingCheck_DB[6]++;
//                  AdhearingCheck_IN[in_negibor]++;
                    if( NMS > maxNMS )
                    {
                        Total_NMS += (NMS-maxNMS);
                        maxNMS = NMS;
                    }
                }
            }
        }

        rIn = p_feavector_db->neighbor_info[minutiae_num_db].dist[7];
        diff_angle = p_feavector_db->neighbor_info[minutiae_num_db].angle[7] - angleIN;
        diff_angle=m_DiffAngleForMatching[diff_angle+360];
        angleLim = m_AngleLimForMatching[rIn];
        if(diff_angle<=angleLim)
        {
            diff_thetta = p_feavector_db->neighbor_info[minutiae_num_db].thetta[7] - thettaIN;
            diff_thetta=m_DiffAngleForMatching[diff_thetta+360];
            tettaLim = TETTA_LIM;
            if(diff_thetta<=tettaLim)
            {
                diff_r = abs(p_feavector_db->neighbor_info[minutiae_num_db].dist[7] - distIN);
                rLim = m_RLimForMatching[rIn];
                if(diff_r<=rLim)
                {
#ifdef  MATCHING_USING_TABLE
                    RP_r =m_RpRForMatching[rIn];
                    RP_angle = m_RpAngleForMatching[rIn];
                    RP_tetta =RP_THETTA;
#else
                    RP_r = P_rMax/rLim;
                    RP_angle = P_rMax/angleLim;
                    RP_tetta = RP_THETTA;
#endif

#ifdef MATCHING_USING_TABLE
    #ifdef WAS_186
                    NMS = (NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + (rIn<<10)/186)>>10;//+ (rIn<<9))>>10;
    #else
                    NMS = (NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + (rIn<<9))>>10;
    #endif
#else
    #ifdef WAS_186
                    NMS = (int)(NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + rIn/186.);
    #else
                    NMS = (int)(NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + rIn*0.5);
    #endif
#endif
                    DIST_PENALTY = p_feavector_db->neighbor_info[minutiae_num_db].dist[7];  //원본이 이건데 버그 같다.
//                  DIST_PENALTY += m_feavector_db->neighbor_info[minutiae_num_db].dist[7];
                    DIST_PENALTY_COUNT++;
//                  AdhearingCheck[in_negibor][7] = NMS;
//                  AdhearingCheck_DB[7]++;
//                  AdhearingCheck_IN[in_negibor]++;
                    if( NMS > maxNMS )
                    {
                        Total_NMS += (NMS-maxNMS);
                        maxNMS = NMS;
                    }
                }
            }
        }
    }
#endif

    if( DIST_PENALTY_COUNT<=0 )
    {
        return 0;
    }

    DIST_PENALTY=DIST_PENALTY/DIST_PENALTY_COUNT;
    if( (DIST_PENALTY>D_DISTANCE_PENALTY && Total_NMS<D_NEIGHBOR_PASS_SCORE2) ||
        (DIST_PENALTY<=D_DISTANCE_PENALTY && Total_NMS<D_NEIGHBOR_PASS_SCORE))
    {
        return 0;
    }

    return Total_NMS;
}

#else

int SCT_Neighbor_Matching_CalculationFORCANDIDATE(int minutiae_num_db,int minutiae_num_in,FeatureVector* p_feavector_db,FeatureVector* p_feavector_in)
{
    //////////////////////////////////////////////////////////////////////////////////////
    /////Penalty Calculation & Calculation of Neighbor Matching Score
    /////
    int i,in_negibor;
    int rIn;
    int rLim,angleLim,tettaLim;
    int RP_r,RP_angle,RP_tetta;
    int NMS;    //NMS=Neighbor Matching Score
    int diff_r,diff_angle,diff_thetta;
    unsigned char AdhearingCheck[D_NEIGHBOR][D_NEIGHBOR];
    unsigned char AdhearingCheck_DB[D_NEIGHBOR]={0};
    unsigned char AdhearingCheck_IN[D_NEIGHBOR]={0};
    int DIST_PENALTY=0,DIST_PENALTY_COUNT=0;
    int Total_NMS;
    int distIN,thettaIN,angleIN;


    memset(AdhearingCheck,0,D_NEIGHBOR*D_NEIGHBOR);
//  memset(AdhearingCheck_DB,0,D_NEIGHBOR);
//  memset(AdhearingCheck_IN,0,D_NEIGHBOR);
    Total_NMS=0;


#if (__SCT_PRODUCT_TYPE__ == __SCT_PRODUCT_MODULE__ || __SCT_PRODUCT_TYPE__ == __SCT_PRODUCT_TnA__)
    for( in_negibor=0; in_negibor<D_NEIGHBOR; in_negibor++ )
    {
        for( i=0; i<D_NEIGHBOR; i++ )
        {
            distIN=p_feavector_in->neighbor_info[minutiae_num_in].dist[in_negibor];
            thettaIN=p_feavector_in->neighbor_info[minutiae_num_in].thetta[in_negibor];
            angleIN=p_feavector_in->neighbor_info[minutiae_num_in].angle[in_negibor];

            rIn = p_feavector_db->neighbor_info[minutiae_num_db].dist[i];
            diff_angle = p_feavector_db->neighbor_info[minutiae_num_db].angle[i] - angleIN;
            diff_angle=DIFFANGLE_FORMATCHING[diff_angle+360];
            angleLim = ANGLELIM_FORMATCHING[rIn];
            if(diff_angle<=angleLim)
            {
                diff_thetta = p_feavector_db->neighbor_info[minutiae_num_db].thetta[i] - thettaIN;
                diff_thetta=DIFFANGLE_FORMATCHING[diff_thetta+360];
                tettaLim = TETTA_LIM;
                if(diff_thetta<=tettaLim)
                {
                    diff_r = abs(p_feavector_db->neighbor_info[minutiae_num_db].dist[i] - distIN);
                    rLim = RLIM_FORMATCHING[rIn];
                    if(diff_r<=rLim)
                    {
                        RP_r = RP_R_FORMATCHING[rIn];
                        RP_angle = RP_ANGLE_FORMATCHING[rIn];
                        RP_tetta = RP_THETTA;
#ifdef WAS_186
                        NMS = (NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + (rIn<<10)/186)>>10;//+ (rIn<<9))>>10;
#else
                        NMS = (NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + (rIn<<9))>>10;
#endif
                        DIST_PENALTY = p_feavector_db->neighbor_info[minutiae_num_db].dist[i];  //원본이 이건데 버그 같다.
//                      DIST_PENALTY += m_feavector_db->neighbor_info[minutiae_num_db].dist[0];
                        DIST_PENALTY_COUNT++;
                        AdhearingCheck[in_negibor][i] = NMS;
                        AdhearingCheck_DB[i]++;
                        AdhearingCheck_IN[in_negibor]++;
                        Total_NMS += NMS;
                    }
                }
            }
        }
    }
#else
    for(in_negibor=0; in_negibor<D_NEIGHBOR; in_negibor++)
    {
        distIN=p_feavector_in->neighbor_info[minutiae_num_in].dist[in_negibor];
        thettaIN=p_feavector_in->neighbor_info[minutiae_num_in].thetta[in_negibor];
        angleIN=p_feavector_in->neighbor_info[minutiae_num_in].angle[in_negibor];

        rIn = p_feavector_db->neighbor_info[minutiae_num_db].dist[0];
        diff_angle = p_feavector_db->neighbor_info[minutiae_num_db].angle[0] - angleIN;
        diff_angle=DIFFANGLE_FORMATCHING[diff_angle+360];
        angleLim = ANGLELIM_FORMATCHING[rIn];
        if(diff_angle<=angleLim)
        {
            diff_thetta = p_feavector_db->neighbor_info[minutiae_num_db].thetta[0] - thettaIN;
            diff_thetta=DIFFANGLE_FORMATCHING[diff_thetta+360];
            tettaLim = TETTA_LIM;
            if(diff_thetta<=tettaLim)
            {
                diff_r = abs(p_feavector_db->neighbor_info[minutiae_num_db].dist[0] - distIN);
                rLim = RLIM_FORMATCHING[rIn];
                if(diff_r<=rLim)
                {
                    RP_r =RP_R_FORMATCHING[rIn];
                    RP_angle = RP_ANGLE_FORMATCHING[rIn];
                    RP_tetta = RP_THETTA;
#ifdef WAS_186
                    NMS = (NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + (rIn<<10)/186)>>10;//+ (rIn<<9))>>10;
#else
                    NMS = (NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + (rIn<<9))>>10;
#endif
                    DIST_PENALTY = p_feavector_db->neighbor_info[minutiae_num_db].dist[0];  //원본이 이건데 버그 같다.
//                  DIST_PENALTY += m_feavector_db->neighbor_info[minutiae_num_db].dist[0];
                    DIST_PENALTY_COUNT++;
                    AdhearingCheck[in_negibor][0] = NMS;
                    AdhearingCheck_DB[0]++;
                    AdhearingCheck_IN[in_negibor]++;
                    Total_NMS += NMS;
                }
            }
        }


        rIn = p_feavector_db->neighbor_info[minutiae_num_db].dist[1];
        diff_angle = p_feavector_db->neighbor_info[minutiae_num_db].angle[1] - angleIN;
        diff_angle=DIFFANGLE_FORMATCHING[diff_angle+360];
        angleLim = ANGLELIM_FORMATCHING[rIn];
        if(diff_angle<=angleLim)
        {
            diff_thetta = p_feavector_db->neighbor_info[minutiae_num_db].thetta[1] - thettaIN;
            diff_thetta=DIFFANGLE_FORMATCHING[diff_thetta+360];
            tettaLim = TETTA_LIM;
            if(diff_thetta<=tettaLim)
            {
                diff_r = abs(p_feavector_db->neighbor_info[minutiae_num_db].dist[1] - distIN);
                rLim = RLIM_FORMATCHING[rIn];
                if(diff_r<=rLim)
                {
                    RP_r =RP_R_FORMATCHING[rIn];
                    RP_angle = RP_ANGLE_FORMATCHING[rIn];
                    RP_tetta = RP_THETTA;
#ifdef WAS_186
                    NMS = (NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + (rIn<<10)/186)>>10;//+ (rIn<<9))>>10;
#else
                    NMS = (NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + (rIn<<9))>>10;
#endif
                    DIST_PENALTY = p_feavector_db->neighbor_info[minutiae_num_db].dist[1];  //원본이 이건데 버그 같다.
//                  DIST_PENALTY += m_feavector_db->neighbor_info[minutiae_num_db].dist[1];
                    DIST_PENALTY_COUNT++;
                    AdhearingCheck[in_negibor][1] = NMS;
                    AdhearingCheck_DB[1]++;
                    AdhearingCheck_IN[in_negibor]++;
                    Total_NMS += NMS;
                }
            }
        }

        rIn = p_feavector_db->neighbor_info[minutiae_num_db].dist[2];
        diff_angle = p_feavector_db->neighbor_info[minutiae_num_db].angle[2] - angleIN;
        diff_angle=DIFFANGLE_FORMATCHING[diff_angle+360];
        angleLim = ANGLELIM_FORMATCHING[rIn];
        if(diff_angle<=angleLim)
        {
            diff_thetta = p_feavector_db->neighbor_info[minutiae_num_db].thetta[2] - thettaIN;
            diff_thetta=DIFFANGLE_FORMATCHING[diff_thetta+360];
            tettaLim = TETTA_LIM;
            if(diff_thetta<=tettaLim)
            {
                diff_r = abs(p_feavector_db->neighbor_info[minutiae_num_db].dist[2] - distIN);
                rLim = RLIM_FORMATCHING[rIn];
                if(diff_r<=rLim)
                {
                    RP_r =RP_R_FORMATCHING[rIn];
                    RP_angle = RP_ANGLE_FORMATCHING[rIn];
                    RP_tetta = RP_THETTA;
#ifdef WAS_186
                    NMS = (NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + (rIn<<10)/186)>>10;//+ (rIn<<9))>>10;
#else
                    NMS = (NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + (rIn<<9))>>10;
#endif
                    DIST_PENALTY = p_feavector_db->neighbor_info[minutiae_num_db].dist[2];  //원본이 이건데 버그 같다.
//                  DIST_PENALTY += m_feavector_db->neighbor_info[minutiae_num_db].dist[2];
                    DIST_PENALTY_COUNT++;
                    AdhearingCheck[in_negibor][2] = NMS;
                    AdhearingCheck_DB[2]++;
                    AdhearingCheck_IN[in_negibor]++;
                    Total_NMS += NMS;
                }
            }
        }

        rIn = p_feavector_db->neighbor_info[minutiae_num_db].dist[3];
        diff_angle = p_feavector_db->neighbor_info[minutiae_num_db].angle[3] - angleIN;
        diff_angle=DIFFANGLE_FORMATCHING[diff_angle+360];
        angleLim = ANGLELIM_FORMATCHING[rIn];
        if(diff_angle<=angleLim)
        {
            diff_thetta = p_feavector_db->neighbor_info[minutiae_num_db].thetta[3] - thettaIN;
            diff_thetta=DIFFANGLE_FORMATCHING[diff_thetta+360];
            tettaLim = TETTA_LIM;
            if(diff_thetta<=tettaLim)
            {
                diff_r = abs(p_feavector_db->neighbor_info[minutiae_num_db].dist[3] - distIN);
                rLim = RLIM_FORMATCHING[rIn];
                if(diff_r<=rLim)
                {
                    RP_r =RP_R_FORMATCHING[rIn];
                    RP_angle = RP_ANGLE_FORMATCHING[rIn];
                    RP_tetta = RP_THETTA;
#ifdef WAS_186
                    NMS = (NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + (rIn<<10)/186)>>10;//+ (rIn<<9))>>10;
#else
                    NMS = (NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + (rIn<<9))>>10;
#endif
                    DIST_PENALTY = p_feavector_db->neighbor_info[minutiae_num_db].dist[3];  //원본이 이건데 버그 같다.
//                  DIST_PENALTY += m_feavector_db->neighbor_info[minutiae_num_db].dist[3];
                    DIST_PENALTY_COUNT++;
                    AdhearingCheck[in_negibor][3] = NMS;
                    AdhearingCheck_DB[3]++;
                    AdhearingCheck_IN[in_negibor]++;
                    Total_NMS += NMS;
                }
            }
        }

        rIn = p_feavector_db->neighbor_info[minutiae_num_db].dist[4];
        diff_angle = p_feavector_db->neighbor_info[minutiae_num_db].angle[4] - angleIN;
        diff_angle=DIFFANGLE_FORMATCHING[diff_angle+360];
        angleLim = ANGLELIM_FORMATCHING[rIn];
        if(diff_angle<=angleLim)
        {
            diff_thetta = p_feavector_db->neighbor_info[minutiae_num_db].thetta[4] - thettaIN;
            diff_thetta=DIFFANGLE_FORMATCHING[diff_thetta+360];
            tettaLim = TETTA_LIM;
            if(diff_thetta<=tettaLim)
            {
                diff_r = abs(p_feavector_db->neighbor_info[minutiae_num_db].dist[4] - distIN);
                rLim = RLIM_FORMATCHING[rIn];
                if(diff_r<=rLim)
                {
                    RP_r =RP_R_FORMATCHING[rIn];
                    RP_angle = RP_ANGLE_FORMATCHING[rIn];
                    RP_tetta = RP_THETTA;
#ifdef WAS_186
                    NMS = (NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + (rIn<<10)/186)>>10;//+ (rIn<<9))>>10;
#else
                    NMS = (NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + (rIn<<9))>>10;
#endif
                    DIST_PENALTY = p_feavector_db->neighbor_info[minutiae_num_db].dist[4];  //원본이 이건데 버그 같다.
//                  DIST_PENALTY += m_feavector_db->neighbor_info[minutiae_num_db].dist[4];
                    DIST_PENALTY_COUNT++;
                    AdhearingCheck[in_negibor][4] = NMS;
                    AdhearingCheck_DB[4]++;
                    AdhearingCheck_IN[in_negibor]++;
                    Total_NMS += NMS;
                }
            }
        }

        rIn = p_feavector_db->neighbor_info[minutiae_num_db].dist[5];
        diff_angle = p_feavector_db->neighbor_info[minutiae_num_db].angle[5] - angleIN;
        diff_angle=DIFFANGLE_FORMATCHING[diff_angle+360];
        angleLim = ANGLELIM_FORMATCHING[rIn];
        if(diff_angle<=angleLim)
        {
            diff_thetta = p_feavector_db->neighbor_info[minutiae_num_db].thetta[5] - thettaIN;
            diff_thetta=DIFFANGLE_FORMATCHING[diff_thetta+360];
            tettaLim = TETTA_LIM;
            if(diff_thetta<=tettaLim)
            {
                diff_r = abs(p_feavector_db->neighbor_info[minutiae_num_db].dist[5] - distIN);
                rLim = RLIM_FORMATCHING[rIn];
                if(diff_r<=rLim)
                {
                    RP_r =RP_R_FORMATCHING[rIn];
                    RP_angle = RP_ANGLE_FORMATCHING[rIn];
                    RP_tetta = RP_THETTA;
#ifdef WAS_186
                    NMS = (NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + (rIn<<10)/186)>>10;//+ (rIn<<9))>>10;
#else
                    NMS = (NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + (rIn<<9))>>10;
#endif
                    DIST_PENALTY = p_feavector_db->neighbor_info[minutiae_num_db].dist[5];  //원본이 이건데 버그 같다.
//                  DIST_PENALTY += m_feavector_db->neighbor_info[minutiae_num_db].dist[5];
                    DIST_PENALTY_COUNT++;
                    AdhearingCheck[in_negibor][5] = NMS;
                    AdhearingCheck_DB[5]++;
                    AdhearingCheck_IN[in_negibor]++;
                    Total_NMS += NMS;
                }
            }
        }

        rIn = p_feavector_db->neighbor_info[minutiae_num_db].dist[6];
        diff_angle = p_feavector_db->neighbor_info[minutiae_num_db].angle[6] - angleIN;
        diff_angle=DIFFANGLE_FORMATCHING[diff_angle+360];
        angleLim = ANGLELIM_FORMATCHING[rIn];
        if(diff_angle<=angleLim)
        {
            diff_thetta = p_feavector_db->neighbor_info[minutiae_num_db].thetta[6] - thettaIN;
            diff_thetta=DIFFANGLE_FORMATCHING[diff_thetta+360];
            tettaLim = TETTA_LIM;
            if(diff_thetta<=tettaLim)
            {
                diff_r = abs(p_feavector_db->neighbor_info[minutiae_num_db].dist[6] - distIN);
                rLim = RLIM_FORMATCHING[rIn];
                if(diff_r<=rLim)
                {
                    RP_r =RP_R_FORMATCHING[rIn];
                    RP_angle = RP_ANGLE_FORMATCHING[rIn];
                    RP_tetta = RP_THETTA;
#ifdef WAS_186
                    NMS = (NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + (rIn<<10)/186)>>10;//+ (rIn<<9))>>10;
#else
                    NMS = (NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + (rIn<<9))>>10;
#endif
                    DIST_PENALTY = p_feavector_db->neighbor_info[minutiae_num_db].dist[6];  //원본이 이건데 버그 같다.
//                  DIST_PENALTY += m_feavector_db->neighbor_info[minutiae_num_db].dist[6];
                    DIST_PENALTY_COUNT++;
                    AdhearingCheck[in_negibor][6] = NMS;
                    AdhearingCheck_DB[6]++;
                    AdhearingCheck_IN[in_negibor]++;
                    Total_NMS += NMS;
                }
            }
        }

        rIn = p_feavector_db->neighbor_info[minutiae_num_db].dist[7];
        diff_angle = p_feavector_db->neighbor_info[minutiae_num_db].angle[7] - angleIN;
        diff_angle=DIFFANGLE_FORMATCHING[diff_angle+360];
        angleLim = ANGLELIM_FORMATCHING[rIn];
        if(diff_angle<=angleLim)
        {
            diff_thetta = p_feavector_db->neighbor_info[minutiae_num_db].thetta[7] - thettaIN;
            diff_thetta=DIFFANGLE_FORMATCHING[diff_thetta+360];
            tettaLim = TETTA_LIM;
            if(diff_thetta<=tettaLim)
            {
                diff_r = abs(p_feavector_db->neighbor_info[minutiae_num_db].dist[7] - distIN);
                rLim = RLIM_FORMATCHING[rIn];
                if(diff_r<=rLim)
                {
                    RP_r =RP_R_FORMATCHING[rIn];
                    RP_angle = RP_ANGLE_FORMATCHING[rIn];
                    RP_tetta = RP_THETTA;
#ifdef WAS_186
                    NMS = (NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + (rIn<<10)/186)>>10;//+ (rIn<<9))>>10;
#else
                    NMS = (NMSmax - RP_r*diff_r - RP_angle*diff_angle - RP_tetta*diff_thetta + (rIn<<9))>>10;
#endif
                    DIST_PENALTY = p_feavector_db->neighbor_info[minutiae_num_db].dist[7];  //원본이 이건데 버그 같다.
//                  DIST_PENALTY += m_feavector_db->neighbor_info[minutiae_num_db].dist[7];
                    DIST_PENALTY_COUNT++;
                    AdhearingCheck[in_negibor][7] = NMS;
                    AdhearingCheck_DB[7]++;
                    AdhearingCheck_IN[in_negibor]++;
                    Total_NMS += NMS;
                }
            }
        }
    }
#endif

    if(DIST_PENALTY_COUNT<=0)
    {
        return 0;
    }
/*  DIST_PENALTY=DIST_PENALTY/DIST_PENALTY_COUNT;
    if( (DIST_PENALTY>D_DISTANCE_PENALTY && Total_NMS<D_NEIGHBOR_PASS_SCORE2) ||
        (DIST_PENALTY<=D_DISTANCE_PENALTY && Total_NMS<D_NEIGHBOR_PASS_SCORE))
    {
        return 0;
    }*/

    NMS = 0;
    for( i=0; i<D_NEIGHBOR; i++ )
    {
        if( AdhearingCheck_DB[i] > 1 || AdhearingCheck_IN[i] > 1 )
        {
            break;
        }
        NMS++;
    }
    if( NMS == D_NEIGHBOR )
        return Total_NMS;

    unsigned char REF_CHECK[D_NEIGHBOR];
    unsigned char INP_CHECK[D_NEIGHBOR];
    int score,MAXScore=0,jj,MAXIndex=0;
    int MAXIndex_i=0,MAXIndex_i2=0,MAXIndex_in_negibor=0,MAXIndex_in_negibor2=0,MAXScore_i,MAXScore_in_negibor;

    Total_NMS = 0;
    memset(REF_CHECK,0,D_NEIGHBOR);
    memset(INP_CHECK,0,D_NEIGHBOR);

    for(in_negibor=0; in_negibor<D_NEIGHBOR; in_negibor++)
    {
        if( AdhearingCheck_IN[in_negibor] == 0 )
            continue;
        for(i=0; i<D_NEIGHBOR; i++)
        {
            if( AdhearingCheck_DB[i] == 0 )
                continue;
            score=AdhearingCheck[in_negibor][i];
            if(score<=0)continue;
            MAXScore=0;
            if(AdhearingCheck_DB[i]==1 && AdhearingCheck_IN[in_negibor]==1)
            {
                if(REF_CHECK[i]==1 ||INP_CHECK[in_negibor]==1)continue;

                Total_NMS += score;
                REF_CHECK[i]=1;
                INP_CHECK[in_negibor]=1;
                AdhearingCheck[in_negibor][i]=0;
                AdhearingCheck_DB[i]--;
                AdhearingCheck_IN[in_negibor]--;
                break;
            }
            else if(AdhearingCheck_DB[i]==1 && AdhearingCheck_IN[in_negibor]>1)
            {
                for(jj=0; jj<D_NEIGHBOR; jj++)
                {
                    score=AdhearingCheck[in_negibor][jj];
                    if(REF_CHECK[jj]==1 ||INP_CHECK[in_negibor]==1)continue;
                    if(score>MAXScore)
                    {
                        MAXScore = score;
                        MAXIndex = jj;
                    }
                }
                if(MAXScore==0)continue;
                REF_CHECK[MAXIndex]=1;
                INP_CHECK[in_negibor]=1;

                AdhearingCheck_DB[i]--;
                AdhearingCheck_IN[in_negibor]--;
                AdhearingCheck[in_negibor][MAXIndex]=0;
                Total_NMS += MAXScore;
                break;
            }
            else if(AdhearingCheck_DB[i]>1 && AdhearingCheck_IN[in_negibor]==1)
            {
                for(jj=0; jj<D_NEIGHBOR; jj++)
                {
                    score=AdhearingCheck[jj][i];
                    if(REF_CHECK[i]==1 ||INP_CHECK[jj]==1)continue;
                    if(score>MAXScore)
                    {
                        MAXScore = score;
                        MAXIndex = jj;
                    }
                }
                if(MAXScore==0)continue;
                REF_CHECK[i]=1;
                INP_CHECK[MAXIndex]=1;
                AdhearingCheck_DB[i]--;
                AdhearingCheck_IN[in_negibor]--;
                AdhearingCheck[MAXIndex][i]=0;
                Total_NMS += MAXScore;
                break;
            }
            else if(AdhearingCheck_DB[i]>1 && AdhearingCheck_IN[in_negibor]>1)
            {
                MAXScore_in_negibor = 0;
                for(jj=0; jj<D_NEIGHBOR; jj++)
                {
                    score=AdhearingCheck[jj][i];
                    if(REF_CHECK[i]==1 ||INP_CHECK[jj]==1)continue;
                    if(score>MAXScore_in_negibor)
                    {
                        MAXScore_in_negibor = score;
                        MAXIndex_in_negibor = jj;
                        MAXIndex_i2 = i;
                    }
                }
                if(MAXScore_in_negibor==0)continue;
                MAXScore_i=0;
                for(jj=0; jj<D_NEIGHBOR; jj++)
                {
                    score=AdhearingCheck[in_negibor][jj];
                    if(REF_CHECK[jj]==1 ||INP_CHECK[in_negibor]==1)continue;
                    if(score>MAXScore_i)
                    {
                        MAXScore_i = score;
                        MAXIndex_i = jj;
                        MAXIndex_in_negibor2=in_negibor;
                    }
                }
                if(MAXScore_i==0)continue;
                if(MAXScore_in_negibor>MAXScore_i)
                {
                    REF_CHECK[MAXIndex_i2]=1;
                    INP_CHECK[MAXIndex_in_negibor]=1;
                    AdhearingCheck_DB[i]--;
                    AdhearingCheck_IN[in_negibor]--;
                    AdhearingCheck[MAXIndex_in_negibor][MAXIndex_i2]=0;
                    Total_NMS += MAXScore_in_negibor;
                    break;
                }
                else
                {
                    REF_CHECK[MAXIndex_i]=1;
                    INP_CHECK[MAXIndex_in_negibor2]=1;
                    AdhearingCheck_DB[i]--;
                    AdhearingCheck_IN[in_negibor]--;
                    AdhearingCheck[MAXIndex_in_negibor2][MAXIndex_i]=0;
                    Total_NMS += MAXScore_i;
                    break;
                }
            }
        }
    }

    //Matched Neighbor calculation
/*  if( (DIST_PENALTY>D_DISTANCE_PENALTY && Total_NMS>=D_NEIGHBOR_PASS_SCORE2) ||
        (DIST_PENALTY<=D_DISTANCE_PENALTY && Total_NMS>=D_NEIGHBOR_PASS_SCORE))
    {
        return Total_NMS;
    }*/
    return 0;
}
#endif

int CIBSMAlgorithm::_FeaGen(unsigned int* feature, FeatureVector* p_feavector, int fea_ver)
{
    if( feature == NULL || p_feavector == NULL )
        return -1;

    int dist, min_dist=MAX_DIAGONAL_LENGTH, max_dist=0;
    int i,j,m,n,x,y,count=0;
    int delta_row,delta_col,neighbor_num,angle,angle_tetta;
    int row,col,dir;
    unsigned char* charFeature=(unsigned char*)feature;
    int remaind_size;

    p_feavector->num = feature[0];
    if( p_feavector->num > MAX_MINUTIAE )
        p_feavector->num = MAX_MINUTIAE;

    //quality
    p_feavector->quality = charFeature[1024];
    p_feavector->core_type = charFeature[1025];
    p_feavector->pitch = charFeature[1026];
    p_feavector->core_dist = charFeature[1027];
    p_feavector->feature_ver = fea_ver;

    if( p_feavector->feature_ver == IBSM_TEMPLATE_VERSION_IBISDK_0 )
    {
        remaind_size = 0;
        p_feavector->quality = 6;
        p_feavector->core_type = 0;
        p_feavector->core_dist = 0;
        p_feavector->pitch = 0;
    }
    else if( p_feavector->feature_ver == IBSM_TEMPLATE_VERSION_IBISDK_1 )
    {
        p_feavector->quality = 6;
        remaind_size = 0;
    }
    else if( p_feavector->feature_ver == IBSM_TEMPLATE_VERSION_IBISDK_2 )
    {
        p_feavector->quality = 6;
        remaind_size = ((MAX_MINUTIAE-p_feavector->num)<<2)>>3;
    }
    else
    {
        // D_FEATURE_VERSION_3 이상
        remaind_size = ((MAX_MINUTIAE-p_feavector->num)<<2)>>3;
    }

    // 계산된 remaind_size 가 feature_num보다 크면 보정해야 함!
    if( remaind_size > p_feavector->num )
        remaind_size = p_feavector->num;

    for(i=1; i<=p_feavector->num; i++)
    {
        p_feavector->type[i-1] = (feature[i]>>31)&0x1;
        p_feavector->x[i-1] = (feature[i]>>20)&0x7FF;
        p_feavector->y[i-1] = (feature[i]>>9)&0x7FF;
        p_feavector->angle[i-1] = (feature[i])&0x1FF;
    }

    //Distance 구하기
    memset(m_DistanceCount,0,sizeof(m_DistanceCount));
    for(i=0; i<p_feavector->num; i++)
    {
        col = p_feavector->x[i];
        row = p_feavector->y[i];
        dir = p_feavector->angle[i];
        for(j=0; j<p_feavector->num; j++)
        {
            if( i == j )
                continue;
            x = abs(col-p_feavector->x[j]);
            y = abs(row-p_feavector->y[j]);
            dist=m_SqrtTable[y][x];
//          dist=(int)sqrt((double)y*y+x*x);
            count=m_DistanceCount[dist];
            m_Distance[dist][count]=j;
            m_DistanceCount[dist]+=1;

#if (__SCT_PRODUCT_TYPE__ == __SCT_PRODUCT_MODULE__ && defined(__BLACKFIN__))
            min_dist = min(dist, min_dist);
            max_dist = max(dist, max_dist);
#else
            if( dist < min_dist ) min_dist = dist;
            else if( dist > max_dist ) max_dist = dist;
#endif
        }
        n=0;
        for(j=min_dist; j<=max_dist; j++)
        {
            count = m_DistanceCount[j];
            if(count<=0)continue;

            for(m=0; m<count; m++)
            {
                neighbor_num = m_Distance[j][m];

                delta_row = row - p_feavector->y[neighbor_num];
                delta_col = col - p_feavector->x[neighbor_num];
                angle = _GetAtanValue(delta_row, delta_col) - dir;
                if (angle<0)
                    angle += 360;

                angle_tetta = p_feavector->angle[neighbor_num]-dir;
                if(angle_tetta<0)
                    angle_tetta += 360;

                p_feavector->neighbor_info[i].angle[n]=angle;
                p_feavector->neighbor_info[i].thetta[n]=angle_tetta;
                p_feavector->neighbor_info[i].dist[n] = j;
                p_feavector->neighbor_info[i].minutiae_num[n] = neighbor_num;

                n++;
                if(n>=D_NEIGHBOR)
                {
                    j=MAX_DIAGONAL_LENGTH;
                    break;
                }
            }
        }
        memset(m_DistanceCount+min_dist,0,max_dist-min_dist+1);
    }

    return 1;
}

void CIBSMAlgorithm::_AlignFeature(FeatureVector *m_fea_db, FeatureVector *m_fea_in, FeatureVector *m_fea_out,
                                  char *MATCEDMINUTIAE_DB,  char *MATCEDMINUTIAE_IN, unsigned short *MATCEDMINUTIAE_SCORE_MINUTIAE,
                                  unsigned char MATCEDMINUTIAE_MATCHEDCOUNT)
{
    int i, SCORE, DB_IDX, IN_IDX;
    double cos_sum, sin_sum, db_cos_sum, db_sin_sum;
    int ScoreSum;
    int DX, DY, CX, CY, ANGLE;
    double radian, cs_radian, sn_radian;
    int MEAN_POS_X_DB, MEAN_POS_Y_DB, MEAN_POS_X_IN, MEAN_POS_Y_IN, MEAN_ANGLE_DB, MEAN_ANGLE_IN;

    memcpy(m_fea_out, m_fea_in, sizeof(FeatureVector));

    if(MATCEDMINUTIAE_MATCHEDCOUNT <= 0)
        return;

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    // 가장 매칭 잘 된 트리의 평균 위치 및 방향 구하기: Weight 중점 구하는 버그 수정
    MEAN_POS_X_DB=0;
    MEAN_POS_Y_DB=0;
    MEAN_POS_X_IN=0;
    MEAN_POS_Y_IN=0;
    cos_sum=1, sin_sum=1;
    db_cos_sum=1, db_sin_sum=1;
    ScoreSum=1;
    for(i=0; i<MATCEDMINUTIAE_MATCHEDCOUNT; i++)
    {
        if(MATCEDMINUTIAE_SCORE_MINUTIAE[i] <= 0)
            continue;

        SCORE = MATCEDMINUTIAE_SCORE_MINUTIAE[i];

        ScoreSum += SCORE;

        DB_IDX = MATCEDMINUTIAE_DB[i];
        IN_IDX = MATCEDMINUTIAE_IN[i];

        MEAN_POS_X_DB += m_fea_db->x[DB_IDX]*SCORE;
        MEAN_POS_Y_DB += m_fea_db->y[DB_IDX]*SCORE;

        MEAN_POS_X_IN += m_fea_in->x[IN_IDX]*SCORE;
        MEAN_POS_Y_IN += m_fea_in->y[IN_IDX]*SCORE;

        db_cos_sum += cs[m_fea_db->angle[DB_IDX]] * SCORE;
        db_sin_sum += sn[m_fea_db->angle[DB_IDX]] * SCORE;

        ANGLE = (m_fea_db->angle[DB_IDX]-m_fea_in->angle[IN_IDX]);
        if(ANGLE < 0) ANGLE += 360;
        cos_sum += cs[ANGLE] * SCORE;
        sin_sum += sn[ANGLE] * SCORE;
    }

    MEAN_POS_X_DB /= ScoreSum;
    MEAN_POS_Y_DB /= ScoreSum;
    MEAN_POS_X_IN /= ScoreSum;
    MEAN_POS_Y_IN /= ScoreSum;
    db_cos_sum = db_cos_sum * 287 / ScoreSum;
    db_sin_sum = db_sin_sum * 287 / ScoreSum;
    cos_sum = cos_sum * 287 / ScoreSum;
    sin_sum = sin_sum * 287 / ScoreSum;
    MEAN_ANGLE_DB = _GetAtanValue((int)db_sin_sum, (int)db_cos_sum);
    ANGLE = _GetAtanValue((int)sin_sum, (int)cos_sum);
    MEAN_ANGLE_IN = MEAN_ANGLE_DB - ANGLE;
    ///////////////////////////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    // Input Feature를 DB Feature에 Align하기
    DX = MEAN_POS_X_DB;
    DY = MEAN_POS_Y_DB;
    CX = MEAN_POS_X_IN;
    CY = MEAN_POS_Y_IN;
    ANGLE = MEAN_ANGLE_DB - MEAN_ANGLE_IN;
    radian = -ANGLE * pi / 180.0;
    cs_radian = cos(radian);
    sn_radian = sin(radian);

    for(i=0; i<m_fea_out->num; i++)
    {
        m_fea_out->angle[i] = m_fea_in->angle[i] + ANGLE;

        if(m_fea_out->angle[i] < 0)
            m_fea_out->angle[i] += 360;
        if(m_fea_out->angle[i] > 360)
            m_fea_out->angle[i] -= 360;

        m_fea_out->x[i] = (int) ( (double)(m_fea_in->x[i]-CX)*cs_radian + (double)(m_fea_in->y[i]-CY)*sn_radian ) + DX;
        m_fea_out->y[i] = (int) ( -(double)(m_fea_in->x[i]-CX)*sn_radian + (double)(m_fea_in->y[i]-CY)*cs_radian ) + DY;
    }
}

int CIBSMAlgorithm::_Area_Calculation(FeatureVector* m_feavector_db,FeatureVector* m_feavector_in,
                                     int* db_back_min_x,int* db_back_min_y,int* db_back_max_x,int* db_back_max_y,
                                     int* in_back_min_x,int* in_back_min_y,int* in_back_max_x,int* in_back_max_y,
                                     int* db_mat_min_x,int* db_mat_min_y,int* db_mat_max_x,int* db_mat_max_y,
                                     int* in_mat_min_x,int* in_mat_min_y,int* in_mat_max_x,int* in_mat_max_y,
                                     int* db_comm_min_x,int* db_comm_min_y,int* db_comm_max_x,int* db_comm_max_y,
                                     int* in_comm_min_x,int* in_comm_min_y,int* in_comm_max_x,int* in_comm_max_y,
                                     char *MATCEDMINUTIAE_DB,
                                     char *MATCEDMINUTIAE_IN,
                                     unsigned short *MATCEDMINUTIAE_SCORE_MINUTIAE,
                                     unsigned char MATCEDMINUTIAE_MATCHEDCOUNT )
{
    ///////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////            영역 구하기
    ///1. 전체 미뉴샤 영역 구하기
    *db_back_min_x=10000;*db_back_min_y=10000;*db_back_max_x=0;*db_back_max_y=0;
    *in_back_min_x=10000;*in_back_min_y=10000;*in_back_max_x=0;*in_back_max_y=0;
    int x,y,i;
    for(i=0; i<m_feavector_db->num; i++)
    {
        x = m_feavector_db->x[i];
        y = m_feavector_db->y[i];
        if(x<*db_back_min_x)
            *db_back_min_x=x;
        if(x>*db_back_max_x)
            *db_back_max_x=x;
        if(y<*db_back_min_y)
            *db_back_min_y=y;
        if(y>*db_back_max_y)
            *db_back_max_y=y;
    }
    for(i=0; i<m_feavector_in->num; i++)
    {
        x = m_feavector_in->x[i];
        y = m_feavector_in->y[i];
        if(x<*in_back_min_x)
            *in_back_min_x=x;
        if(x>*in_back_max_x)
            *in_back_max_x=x;
        if(y<*in_back_min_y)
            *in_back_min_y=y;
        if(y>*in_back_max_y)
            *in_back_max_y=y;
    }
    ///2. 메칭된 미뉴샤 영역구하기
    *db_mat_min_x=10000;*db_mat_min_y=10000;*db_mat_max_x=0;*db_mat_max_y=0;
    *in_mat_min_x=10000;*in_mat_min_y=10000;*in_mat_max_x=0;*in_mat_max_y=0;
    int matched_count_maxtree=MATCEDMINUTIAE_MATCHEDCOUNT,Ref,Inp;
    for(i=0; i<matched_count_maxtree; i++)
    {
        if(MATCEDMINUTIAE_SCORE_MINUTIAE[i] <= 0)
            continue;

        Ref = MATCEDMINUTIAE_DB[i];
        x = m_feavector_db->x[Ref];
        y = m_feavector_db->y[Ref];
        if(x<*db_mat_min_x)
            *db_mat_min_x=x;
        if(x>*db_mat_max_x)
            *db_mat_max_x=x;
        if(y<*db_mat_min_y)
            *db_mat_min_y=y;
        if(y>*db_mat_max_y)
            *db_mat_max_y=y;
    }
    for(i=0; i<matched_count_maxtree; i++)
    {
        if(MATCEDMINUTIAE_SCORE_MINUTIAE[i] <= 0)
            continue;

        Inp = MATCEDMINUTIAE_IN[i];
        x = m_feavector_in->x[Inp];
        y = m_feavector_in->y[Inp];
        if(x<*in_mat_min_x)
            *in_mat_min_x=x;
        if(x>*in_mat_max_x)
            *in_mat_max_x=x;
        if(y<*in_mat_min_y)
            *in_mat_min_y=y;
        if(y>*in_mat_max_y)
            *in_mat_max_y=y;
    }
    ///3. 공통영역 구하기
    *db_comm_min_x=10000;*db_comm_min_y=10000;*db_comm_max_x=0;*db_comm_max_y=0;
    *in_comm_min_x=10000;*in_comm_min_y=10000;*in_comm_max_x=0;*in_comm_max_y=0;
    int diff_db,diff_in;
    diff_db = *db_mat_min_x-*db_back_min_x;
    diff_in = *in_mat_min_x-*in_back_min_x;
    if(diff_db<=diff_in)
    {
        *db_comm_min_x = *db_mat_min_x-diff_db;
        *in_comm_min_x = *in_mat_min_x-diff_db;
    }
    else
    {
        *db_comm_min_x = *db_mat_min_x-diff_in;
        *in_comm_min_x = *in_mat_min_x-diff_in;
    }
    diff_db = *db_back_max_x-*db_mat_max_x;
    diff_in = *in_back_max_x-*in_mat_max_x;
    if(diff_db<=diff_in)
    {
        *db_comm_max_x = *db_mat_max_x+diff_db;
        *in_comm_max_x = *in_mat_max_x+diff_db;
    }
    else
    {
        *db_comm_max_x = *db_mat_max_x+diff_in;
        *in_comm_max_x = *in_mat_max_x+diff_in;
    }

    diff_db = *db_mat_min_y-*db_back_min_y;
    diff_in = *in_mat_min_y-*in_back_min_y;
    if(diff_db<=diff_in)
    {
        *db_comm_min_y = *db_mat_min_y-diff_db;
        *in_comm_min_y = *in_mat_min_y-diff_db;
    }
    else
    {
        *db_comm_min_y = *db_mat_min_y-diff_in;
        *in_comm_min_y = *in_mat_min_y-diff_in;
    }
    diff_db = *db_back_max_y-*db_mat_max_y;
    diff_in = *in_back_max_y-*in_mat_max_y;
    if(diff_db<=diff_in)
    {
        *db_comm_max_y = *db_mat_max_y+diff_db;
        *in_comm_max_y = *in_mat_max_y+diff_db;
    }
    else
    {
        *db_comm_max_y = *db_mat_max_y+diff_in;
        *in_comm_max_y = *in_mat_max_y+diff_in;
    }

    return 1;
}

int CIBSMAlgorithm::_Calculate_Minutiae_NeighborDist_New(FeatureVector *m_feavector_db, FeatureVector *m_feavector_in,
                                    /*unsigned char (*DB_DISTINFO)[100],unsigned char (*IN_DISTINFO)[100],*/

                                    int db_comm_min_x,int db_comm_max_x,int db_comm_min_y,int db_comm_max_y,
                                    int db_mat_min_x,int db_mat_max_x,int db_mat_min_y,int db_mat_max_y,

                                    int in_comm_min_x,int in_comm_max_x,int in_comm_min_y,int in_comm_max_y,
                                    int in_mat_min_x,int in_mat_max_x,int in_mat_min_y,int in_mat_max_y,

                                    int* db_goodquality_nonmatchedminutiae_num_in_commarea,int* db_goodquality_nonmatchedminutiae_num_in_matarea,
                                    int* in_goodquality_nonmatchedminutiae_num_in_commarea,int* in_goodquality_nonmatchedminutiae_num_in_matarea,

                                    int* db_badquality_nonmatchedminutiae_num_in_commarea,int* db_badquality_nonmatchedminutiae_num_in_matarea,
                                    int* in_badquality_nonmatchedminutiae_num_in_commarea,int* in_badquality_nonmatchedminutiae_num_in_matarea,

                                    int* db_goodquality_matchedminutiae_num_in_commarea,int* db_goodquality_matchedminutiae_num_in_matarea,
                                    int* in_goodquality_matchedminutiae_num_in_commarea,int* in_goodquality_matchedminutiae_num_in_matarea,

                                    int* db_badquality_matchedminutiae_num_in_commarea,int* db_badquality_matchedminutiae_num_in_matarea,
                                    int* in_badquality_matchedminutiae_num_in_commarea,int* in_badquality_matchedminutiae_num_in_matarea,

                                    int* db_minutiae_totalnum_in_commarea,int* db_minutiae_totalnum_in_matarea,
                                    int* in_minutiae_totalnum_in_commarea,int* in_minutiae_totalnum_in_matarea,

                                    char *MATCEDMINUTIAE_DB,
                                    char *MATCEDMINUTIAE_IN,
                                    unsigned short *MATCEDMINUTIAE_SCORE_MINUTIAE,
                                    unsigned char MATCEDMINUTIAE_MATCHEDCOUNT,
                                    unsigned char *Minutiae_Quality_DB,
                                    unsigned char *Minutiae_Quality_IN)
{
    int Ref_Num,Inp_Num;
    int i,j,dist;
    int x,y;
    int MATDB[MAX_MINUTIAE];
    int MATIN[MAX_MINUTIAE];

    //초기화
    memset(MATDB,0,sizeof(MATDB));
    memset(MATIN,0,sizeof(MATIN));
    memset(Minutiae_Quality_DB,0,MAX_MINUTIAE);
    memset(Minutiae_Quality_IN,0,MAX_MINUTIAE);
    *db_goodquality_nonmatchedminutiae_num_in_commarea=0;*db_goodquality_nonmatchedminutiae_num_in_matarea=0;
    *in_goodquality_nonmatchedminutiae_num_in_commarea=0;*in_goodquality_nonmatchedminutiae_num_in_matarea=0;
    *db_badquality_nonmatchedminutiae_num_in_commarea=0;*db_badquality_nonmatchedminutiae_num_in_matarea=0;
    *in_badquality_nonmatchedminutiae_num_in_commarea=0;*in_badquality_nonmatchedminutiae_num_in_matarea=0;
    *db_goodquality_matchedminutiae_num_in_commarea=0;*db_goodquality_matchedminutiae_num_in_matarea=0;
    *in_goodquality_matchedminutiae_num_in_commarea=0;*in_goodquality_matchedminutiae_num_in_matarea=0;
    *db_badquality_matchedminutiae_num_in_commarea=0;*db_badquality_matchedminutiae_num_in_matarea=0;
    *in_badquality_matchedminutiae_num_in_commarea=0;*in_badquality_matchedminutiae_num_in_matarea=0;
    *db_minutiae_totalnum_in_commarea=0;*db_minutiae_totalnum_in_matarea=0;
    *in_minutiae_totalnum_in_commarea=0;*in_minutiae_totalnum_in_matarea=0;

    //ordering matched minutiae DB, IN
    for(i=0; i<MATCEDMINUTIAE_MATCHEDCOUNT; i++)
    {
        if(MATCEDMINUTIAE_SCORE_MINUTIAE[i] <= 0)
            continue;

        Ref_Num = MATCEDMINUTIAE_DB[i];
        Inp_Num = MATCEDMINUTIAE_IN[i];
        MATDB[Ref_Num] = 1;
        MATIN[Inp_Num] = 1;
    }

    Ref_Num = m_feavector_db->num;
    Inp_Num = m_feavector_in->num;

    //db_minutiae_totalnum_in_commarea,db_minutiae_totalnum_in_matarea 계산
    for(i=0; i<Ref_Num; i++)
    {
        x = m_feavector_db->x[i];
        y = m_feavector_db->y[i];
        if(x>=db_comm_min_x && x<=db_comm_max_x && y>=db_comm_min_y && y<=db_comm_max_y)
            (*db_minutiae_totalnum_in_commarea)++;
        if(x>=db_mat_min_x && x<=db_mat_max_x && y>=db_mat_min_y && y<=db_mat_max_y)
            (*db_minutiae_totalnum_in_matarea)++;
    }
    //in_minutiae_totalnum_in_commarea,in_minutiae_totalnum_in_matarea 계산
    for(i=0; i<Inp_Num; i++)
    {
        x = m_feavector_in->x[i];
        y = m_feavector_in->y[i];
        if(x>=in_comm_min_x && x<=in_comm_max_x && y>=in_comm_min_y && y<=in_comm_max_y)
            (*in_minutiae_totalnum_in_commarea)++;
        if(x>=in_mat_min_x && x<=in_mat_max_x && y>=in_mat_min_y && y<=in_mat_max_y)
            (*in_minutiae_totalnum_in_matarea)++;
    }

    //DB
    int Neighbor=4,BAD_TRES=20,NeighborBADMCount=5;
    int GOOD_TRES=30;
    int dist_low;
    for(i=0; i<m_feavector_db->num; i++)
    {
        if(MATDB[i]>0)      //matched minutiae
        {
            dist_low=0;
            for(j=0; j<Neighbor; j++)
            {
                dist_low+=m_feavector_db->neighbor_info[i].dist[j];
            }
            dist_low=dist_low/Neighbor;
            if(dist_low<BAD_TRES)
            {
                x = m_feavector_db->x[i];
                y = m_feavector_db->y[i];
                if(x>=db_comm_min_x && x<=db_comm_max_x && y>=db_comm_min_y && y<=db_comm_max_y)
                    (*db_badquality_matchedminutiae_num_in_commarea)++;
                if(x>=db_mat_min_x && x<=db_mat_max_x && y>=db_mat_min_y && y<=db_mat_max_y)
                    (*db_badquality_matchedminutiae_num_in_matarea)++;
                Minutiae_Quality_DB[i]=1;
            }
            else
            {
                x = m_feavector_db->x[i];
                y = m_feavector_db->y[i];
                if(x>=db_comm_min_x && x<=db_comm_max_x && y>=db_comm_min_y && y<=db_comm_max_y)
                    (*db_goodquality_matchedminutiae_num_in_commarea)++;
                if(x>=db_mat_min_x && x<=db_mat_max_x && y>=db_mat_min_y && y<=db_mat_max_y)
                    (*db_goodquality_matchedminutiae_num_in_matarea)++;
                Minutiae_Quality_DB[i]=100;
            }
        }
        else        //non matched minutiae
        {
            dist_low=0;
            for(j=0; j<Neighbor; j++)
            {
                dist_low+=m_feavector_db->neighbor_info[i].dist[j];
            }
            dist_low=dist_low/Neighbor;
            if(dist_low<BAD_TRES)
            {
                x = m_feavector_db->x[i];
                y = m_feavector_db->y[i];
                if(x>=db_comm_min_x && x<=db_comm_max_x && y>=db_comm_min_y && y<=db_comm_max_y)
                    (*db_badquality_nonmatchedminutiae_num_in_commarea)++;
                if(x>=db_mat_min_x && x<=db_mat_max_x && y>=db_mat_min_y && y<=db_mat_max_y)
                    (*db_badquality_nonmatchedminutiae_num_in_matarea)++;
                Minutiae_Quality_DB[i]=1;
            }
            else
            {
                x = m_feavector_db->x[i];
                y = m_feavector_db->y[i];
                if(x>=db_comm_min_x && x<=db_comm_max_x && y>=db_comm_min_y && y<=db_comm_max_y)
                    (*db_goodquality_nonmatchedminutiae_num_in_commarea)++;
                if(x>=db_mat_min_x && x<=db_mat_max_x && y>=db_mat_min_y && y<=db_mat_max_y)
                    (*db_goodquality_nonmatchedminutiae_num_in_matarea)++;
                Minutiae_Quality_DB[i]=100;
            }
        }
    }

    int badMCount,Ref;
    for(i=0; i<m_feavector_db->num; i++)
    {
        if(Minutiae_Quality_DB[i]<100)continue;
        badMCount=0;
        dist=0;
        for(j=0; j<D_NEIGHBOR; j++)
        {
            Ref = m_feavector_db->neighbor_info[i].minutiae_num[j];
            if(Minutiae_Quality_DB[Ref]==1)
                badMCount++;
            // bug fixed
            dist+=m_feavector_db->neighbor_info[i].dist[j];//dist+=m_feavector_db->neighbor_info[i].minutiae_num[j];
        }
        dist=dist/D_NEIGHBOR;
        if(badMCount>NeighborBADMCount && dist<GOOD_TRES)
        {
            Minutiae_Quality_DB[i]=1;
            x = m_feavector_db->x[i];
            y = m_feavector_db->y[i];
            if(x>=db_mat_min_x && x<=db_mat_max_x && y>=db_mat_min_y && y<=db_mat_max_y)
            {
                if(MATDB[i]>0)
                {
                    (*db_badquality_matchedminutiae_num_in_matarea)++;
                    (*db_badquality_matchedminutiae_num_in_commarea)++;
                    (*db_goodquality_matchedminutiae_num_in_matarea)--;
                    (*db_goodquality_matchedminutiae_num_in_commarea)--;
                }
                else
                {
                    (*db_badquality_nonmatchedminutiae_num_in_matarea)++;
                    (*db_badquality_nonmatchedminutiae_num_in_commarea)++;
                    (*db_goodquality_nonmatchedminutiae_num_in_matarea)--;
                    (*db_goodquality_nonmatchedminutiae_num_in_commarea)--;
                }
            }
            else if(x>=db_comm_min_x && x<=db_comm_max_x && y>=db_comm_min_y && y<=db_comm_max_y)
            {
                (*db_badquality_nonmatchedminutiae_num_in_commarea)++;
                (*db_goodquality_nonmatchedminutiae_num_in_commarea)--;
            }
        }
    }

    //Input
    for(i=0; i<m_feavector_in->num; i++)
    {
        if(MATIN[i]>0)      //matched minutiae
        {
            dist_low=0;
            for(j=0; j<Neighbor; j++)
            {
                dist_low+=m_feavector_in->neighbor_info[i].dist[j];
            }
            dist_low=dist_low/Neighbor;
            if(dist_low<BAD_TRES)
            {
                x = m_feavector_in->x[i];
                y = m_feavector_in->y[i];
                if(x>=in_comm_min_x && x<=in_comm_max_x && y>=in_comm_min_y && y<=in_comm_max_y)
                    (*in_badquality_matchedminutiae_num_in_commarea)++;
                if(x>=in_mat_min_x && x<=in_mat_max_x && y>=in_mat_min_y && y<=in_mat_max_y)
                    (*in_badquality_matchedminutiae_num_in_matarea)++;
                Minutiae_Quality_IN[i]=1;
            }
            else
            {
                x = m_feavector_in->x[i];
                y = m_feavector_in->y[i];
                if(x>=in_comm_min_x && x<=in_comm_max_x && y>=in_comm_min_y && y<=in_comm_max_y)
                    (*in_goodquality_matchedminutiae_num_in_commarea)++;
                if(x>=in_mat_min_x && x<=in_mat_max_x && y>=in_mat_min_y && y<=in_mat_max_y)
                    (*in_goodquality_matchedminutiae_num_in_matarea)++;
                Minutiae_Quality_IN[i]=100;
            }
        }
        else        //non matched minutiae
        {
            dist_low=0;
            for(j=0; j<Neighbor; j++)
            {
                dist_low+=m_feavector_in->neighbor_info[i].dist[j];
            }
            dist_low=dist_low/Neighbor;
            if(dist_low<BAD_TRES)
            {
                x = m_feavector_in->x[i];
                y = m_feavector_in->y[i];
                if(x>=in_comm_min_x && x<=in_comm_max_x && y>=in_comm_min_y && y<=in_comm_max_y)
                    (*in_badquality_nonmatchedminutiae_num_in_commarea)++;
                if(x>=in_mat_min_x && x<=in_mat_max_x && y>=in_mat_min_y && y<=in_mat_max_y)
                    (*in_badquality_nonmatchedminutiae_num_in_matarea)++;
                Minutiae_Quality_IN[i]=1;
            }
            else
            {
                x = m_feavector_in->x[i];
                y = m_feavector_in->y[i];
                if(x>=in_comm_min_x && x<=in_comm_max_x && y>=in_comm_min_y && y<=in_comm_max_y)
                    (*in_goodquality_nonmatchedminutiae_num_in_commarea)++;
                if(x>=in_mat_min_x && x<=in_mat_max_x && y>=in_mat_min_y && y<=in_mat_max_y)
                    (*in_goodquality_nonmatchedminutiae_num_in_matarea)++;
                Minutiae_Quality_IN[i]=100;
            }
        }
    }

    int Inp;
    for(i=0; i<m_feavector_in->num; i++)
    {
        if(Minutiae_Quality_IN[i]<100)continue;
        badMCount=0;
        dist=0;
        for(j=0; j<D_NEIGHBOR; j++)
        {
            Inp = m_feavector_in->neighbor_info[i].minutiae_num[j];
            if(Minutiae_Quality_IN[Inp]==1)
                badMCount++;
            // bug fixed
            dist+=m_feavector_in->neighbor_info[i].dist[j];//dist+=m_feavector_in->neighbor_info[i].minutiae_num[j];
        }
        dist=dist/D_NEIGHBOR;
        if(badMCount>NeighborBADMCount && dist<GOOD_TRES)
        {
            Minutiae_Quality_IN[i]=1;
            x = m_feavector_in->x[i];
            y = m_feavector_in->y[i];
            if(x>=in_mat_min_x && x<=in_mat_max_x && y>=in_mat_min_y && y<=in_mat_max_y)
            {
                if(MATIN[i]>0)
                {
                    (*in_badquality_matchedminutiae_num_in_matarea)++;
                    (*in_badquality_matchedminutiae_num_in_commarea)++;
                    (*in_goodquality_matchedminutiae_num_in_matarea)--;
                    (*in_goodquality_matchedminutiae_num_in_commarea)--;
                }
                else
                {
                    (*in_badquality_nonmatchedminutiae_num_in_matarea)++;
                    (*in_badquality_nonmatchedminutiae_num_in_commarea)++;
                    (*in_goodquality_nonmatchedminutiae_num_in_matarea)--;
                    (*in_goodquality_nonmatchedminutiae_num_in_commarea)--;
                }
            }
            else if(x>=in_comm_min_x && x<=in_comm_max_x && y>=in_comm_min_y && y<=in_comm_max_y)
            {
                (*in_badquality_nonmatchedminutiae_num_in_commarea)++;
                (*in_goodquality_nonmatchedminutiae_num_in_commarea)--;
            }
        }
    }
    return 1;
}

int CIBSMAlgorithm::_PenaltyCalc(FeatureVector* p_feavector_db, FeatureVector* p_feavector_in, int Total_NMS,
                                char *STEP_MATCEDMINUTIAE_DB, char *STEP_MATCEDMINUTIAE_IN,
                                unsigned short *STEP_MATCEDMINUTIAE_SCORE_MINUTIAE, unsigned char MatchedMinutiaCount)
{
    int Gdb_goodquality_nonmatchedminutiae_num_in_commarea,Gdb_goodquality_nonmatchedminutiae_num_in_matarea;
    int Gin_goodquality_nonmatchedminutiae_num_in_commarea,Gin_goodquality_nonmatchedminutiae_num_in_matarea;
    int Gdb_badquality_nonmatchedminutiae_num_in_commarea,Gdb_badquality_nonmatchedminutiae_num_in_matarea;
    int Gin_badquality_nonmatchedminutiae_num_in_commarea,Gin_badquality_nonmatchedminutiae_num_in_matarea;
    int Gdb_goodquality_matchedminutiae_num_in_commarea,Gdb_goodquality_matchedminutiae_num_in_matarea;
    int Gin_goodquality_matchedminutiae_num_in_commarea,Gin_goodquality_matchedminutiae_num_in_matarea;
    int Gdb_badquality_matchedminutiae_num_in_commarea,Gdb_badquality_matchedminutiae_num_in_matarea;
    int Gin_badquality_matchedminutiae_num_in_commarea,Gin_badquality_matchedminutiae_num_in_matarea;
    int Gdb_minutiae_totalnum_in_commarea,Gdb_minutiae_totalnum_in_matarea;
    int Gin_minutiae_totalnum_in_commarea,Gin_minutiae_totalnum_in_matarea;

    ///1. 전체 미뉴샤 영역 구하기
    int db_back_min_x,db_back_min_y,db_back_max_x,db_back_max_y;
    int in_back_min_x,in_back_min_y,in_back_max_x,in_back_max_y;
    ///2. 메칭된 미뉴샤 영역구하기
    int db_mat_min_x,db_mat_min_y,db_mat_max_x,db_mat_max_y;
    int in_mat_min_x,in_mat_min_y,in_mat_max_x,in_mat_max_y;
    ///3. 공통영역 구하기
    int db_comm_min_x,db_comm_min_y,db_comm_max_x,db_comm_max_y;
    int in_comm_min_x,in_comm_min_y,in_comm_max_x,in_comm_max_y;

    unsigned char Minutiae_Quality_DB[MAX_MINUTIAE];
    unsigned char Minutiae_Quality_IN[MAX_MINUTIAE];

	memset(Minutiae_Quality_DB, 0, sizeof(Minutiae_Quality_DB));
	memset(Minutiae_Quality_IN, 0, sizeof(Minutiae_Quality_IN));

    // D_DIRECT_MATCHING_SCORE 이상은 해볼 필요 없음
    if(Total_NMS >= D_DIRECT_MATCHING_SCORE)
        return Total_NMS;

    Gdb_goodquality_nonmatchedminutiae_num_in_commarea=Gdb_goodquality_nonmatchedminutiae_num_in_matarea=0;
    Gin_goodquality_nonmatchedminutiae_num_in_commarea=Gin_goodquality_nonmatchedminutiae_num_in_matarea=0;
    Gdb_badquality_nonmatchedminutiae_num_in_commarea=Gdb_badquality_nonmatchedminutiae_num_in_matarea=0;
    Gin_badquality_nonmatchedminutiae_num_in_commarea=Gin_badquality_nonmatchedminutiae_num_in_matarea=0;
    Gdb_goodquality_matchedminutiae_num_in_commarea=Gdb_goodquality_matchedminutiae_num_in_matarea=0;
    Gin_goodquality_matchedminutiae_num_in_commarea=Gin_goodquality_matchedminutiae_num_in_matarea=0;
    Gdb_badquality_matchedminutiae_num_in_commarea=Gdb_badquality_matchedminutiae_num_in_matarea=0;
    Gin_badquality_matchedminutiae_num_in_commarea=Gin_badquality_matchedminutiae_num_in_matarea=0;
    Gdb_minutiae_totalnum_in_commarea=Gdb_minutiae_totalnum_in_matarea=0;
    Gin_minutiae_totalnum_in_commarea=Gin_minutiae_totalnum_in_matarea=0;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Penalty 계산 시 필요한 정보 미리 계산
    int i, j, count;
//  unsigned char DB_DISTINFO[MAX_MINUTIAE][100];
//  unsigned char IN_DISTINFO[MAX_MINUTIAE][100];
    FeatureVector align_in;
    memset(&align_in, 0, sizeof(FeatureVector));
    _AlignFeature(p_feavector_db, p_feavector_in, &align_in, STEP_MATCEDMINUTIAE_DB,
                STEP_MATCEDMINUTIAE_IN, STEP_MATCEDMINUTIAE_SCORE_MINUTIAE, MatchedMinutiaCount);

    _Area_Calculation(p_feavector_db,&align_in,
                    &db_back_min_x, &db_back_min_y, &db_back_max_x, &db_back_max_y,
                    &in_back_min_x, &in_back_min_y, &in_back_max_x, &in_back_max_y,
                    &db_mat_min_x, &db_mat_min_y, &db_mat_max_x, &db_mat_max_y,
                    &in_mat_min_x, &in_mat_min_y, &in_mat_max_x, &in_mat_max_y,
                    &db_comm_min_x, &db_comm_min_y, &db_comm_max_x, &db_comm_max_y,
                    &in_comm_min_x, &in_comm_min_y, &in_comm_max_x, &in_comm_max_y,
                    STEP_MATCEDMINUTIAE_DB,
                    STEP_MATCEDMINUTIAE_IN,
                    STEP_MATCEDMINUTIAE_SCORE_MINUTIAE,
                    MatchedMinutiaCount);

    _Calculate_Minutiae_NeighborDist_New(p_feavector_db, &align_in,/*DB_DISTINFO,IN_DISTINFO,*/
                db_comm_min_x,db_comm_max_x,db_comm_min_y,db_comm_max_y,
                db_mat_min_x,db_mat_max_x,db_mat_min_y,db_mat_max_y,
                in_comm_min_x,in_comm_max_x,in_comm_min_y,in_comm_max_y,
                in_mat_min_x,in_mat_max_x,in_mat_min_y,in_mat_max_y,
                &Gdb_goodquality_nonmatchedminutiae_num_in_commarea,&Gdb_goodquality_nonmatchedminutiae_num_in_matarea,
                &Gin_goodquality_nonmatchedminutiae_num_in_commarea,&Gin_goodquality_nonmatchedminutiae_num_in_matarea,
                &Gdb_badquality_nonmatchedminutiae_num_in_commarea,&Gdb_badquality_nonmatchedminutiae_num_in_matarea,
                &Gin_badquality_nonmatchedminutiae_num_in_commarea,&Gin_badquality_nonmatchedminutiae_num_in_matarea,
                &Gdb_goodquality_matchedminutiae_num_in_commarea,&Gdb_goodquality_matchedminutiae_num_in_matarea,
                &Gin_goodquality_matchedminutiae_num_in_commarea,&Gin_goodquality_matchedminutiae_num_in_matarea,
                &Gdb_badquality_matchedminutiae_num_in_commarea,&Gdb_badquality_matchedminutiae_num_in_matarea,
                &Gin_badquality_matchedminutiae_num_in_commarea,&Gin_badquality_matchedminutiae_num_in_matarea,
                &Gdb_minutiae_totalnum_in_commarea,&Gdb_minutiae_totalnum_in_matarea,
                &Gin_minutiae_totalnum_in_commarea,&Gin_minutiae_totalnum_in_matarea,
                STEP_MATCEDMINUTIAE_DB,
                STEP_MATCEDMINUTIAE_IN,
                STEP_MATCEDMINUTIAE_SCORE_MINUTIAE,
                MatchedMinutiaCount,
                Minutiae_Quality_DB,
                Minutiae_Quality_IN);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // UK Penalty
    {
        int inp=0, idx, matched_flag;
        int x, y, tmp_count, tmp_count2, NM_IL, M_IL;
        int TotalNeighborCnt;
        int Penalty_1, Penalty_2;
        NM_IL = M_IL = 0;
        for(i=0; i<p_feavector_db->num; i++)
        {
            x = p_feavector_db->x[i];
            y = p_feavector_db->y[i];
            if(db_comm_min_x > x || db_comm_max_x < x || db_comm_min_y > y || db_comm_max_y < y)
                continue;

            matched_flag = 0;
            for(j=0; j<MatchedMinutiaCount; j++)
            {
                if(STEP_MATCEDMINUTIAE_SCORE_MINUTIAE[j] <= 0)
                    continue;

                if(i == STEP_MATCEDMINUTIAE_DB[j])
                {
                    inp = j;
                    matched_flag = 1;
                    break;
                }
            }

            if(matched_flag == 1)
            {
                tmp_count=tmp_count2=0;
                for(j=0; j<D_NEIGHBOR; j++)
                {
                    idx = p_feavector_db->neighbor_info[i].minutiae_num[j];
                    x = p_feavector_db->x[idx];
                    y = p_feavector_db->y[idx];

                    if(db_comm_min_x>x || db_comm_max_x < x || db_comm_min_y > y || db_comm_max_y < y)
                        tmp_count++;

                    idx = align_in.neighbor_info[STEP_MATCEDMINUTIAE_IN[inp]].minutiae_num[j];
                    x = align_in.x[idx];
                    y = align_in.y[idx];

                    if(in_comm_min_x > x || in_comm_max_x < x || in_comm_min_y > y || in_comm_max_y < y)
                        tmp_count2++;
                }
                if(tmp_count < tmp_count2)
                    tmp_count = tmp_count2;

                M_IL += tmp_count;
            }
            else
            {
                tmp_count=0;
                for(j=0; j<D_NEIGHBOR; j++)
                {
                    idx = p_feavector_db->neighbor_info[i].minutiae_num[j];
                    x = p_feavector_db->x[idx];
                    y = p_feavector_db->y[idx];

                    if(db_comm_min_x > x || db_comm_max_x < x || db_comm_min_y > y || db_comm_max_y < y)
                        tmp_count++;
                }
                if(tmp_count >= D_NEIGHBOR-2)   // 최소 6개 이상이 CA 밖의 Neighbor를 가진 상태라면
                    NM_IL++;
            }
        }

        // + penalty
        TotalNeighborCnt = MatchedMinutiaCount * D_NEIGHBOR;
        Penalty_1 = (M_IL * Total_NMS) / (TotalNeighborCnt*2);  //ImpossibleLinkScore

        // - penalty
        Penalty_2 = (Gdb_minutiae_totalnum_in_commarea-MatchedMinutiaCount-NM_IL)*13;       //CommonAreaScore

        // 특징점 분포에 따른 패널티
        int minutia_rate,Penalty_3;
        minutia_rate = (db_back_max_x-db_back_min_x)*(db_back_max_y-db_back_min_y)/p_feavector_db->num;
        minutia_rate += (in_back_max_x-in_back_min_x)*(in_back_max_y-in_back_min_y)/p_feavector_in->num;
        minutia_rate/=2;
//      Penalty_3 = (minutia_rate * Total_NMS >> 13) - (Total_NMS >> 3);
        Penalty_3 = (int)( ( ( minutia_rate / (float)(65<<4) ) * Total_NMS - Total_NMS) * 18. / 128 );

        Total_NMS = Total_NMS + (Penalty_1 + Penalty_3 - Penalty_2);
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Penalty - 특징점 3개 정도 코어 매칭
    {
        int db_idx1, db_idx2, dx, dy;
        int distance, Penalty_Core=0, Penalty_Core_Score=0;
        for(i=0; i<MatchedMinutiaCount; i++)
        {
            if(STEP_MATCEDMINUTIAE_SCORE_MINUTIAE[i] <= 0)
                continue;

            db_idx1 = STEP_MATCEDMINUTIAE_DB[i];

            count=1;
            for(j=0; j<MatchedMinutiaCount; j++)
            {
                if(STEP_MATCEDMINUTIAE_SCORE_MINUTIAE[j] <= 0)
                    continue;

                db_idx2 = STEP_MATCEDMINUTIAE_DB[j];

                if(db_idx1 == db_idx2) continue;

                dx = abs(p_feavector_db->x[db_idx1] - p_feavector_db->x[db_idx2]);
                dy = abs(p_feavector_db->y[db_idx1] - p_feavector_db->y[db_idx2]);
                distance = m_SqrtTable[dy][dx];
//              distance = (int)sqrt((double)dy*dy+dx*dx);
                if(distance < 25)
                    count++;
            }
            if(count >= 3)
            {
                if(Penalty_Core < count)
                    Penalty_Core = count;
            }
        }

//      Total_NMS = Total_NMS - Penalty_1*D_NEIGHBOR_PASS_SCORE;
        if(Penalty_Core >= 2)
        {
//          Penalty_Core-=2;

            int CoreMinutiaScore = Total_NMS / MatchedMinutiaCount;// + Gdb_goodquality_nonmatchedminutiae_num_in_commarea/3);
//          if(CoreMinutiaScore > D_NEIGHBOR_PASS_SCORE)
//              CoreMinutiaScore = D_NEIGHBOR_PASS_SCORE;
            Penalty_Core_Score = CoreMinutiaScore * Penalty_Core;
//          Penalty_Core_Score = Penalty_Core_Score - Penalty_Core_Score * (MatchedMinutiaCount - Penalty_Core) / (MatchedMinutiaCount*2);
            Total_NMS -= Penalty_Core_Score;
        }
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Penalty - Knuckle
    {
        FeatureVector *p_feavector_in = &align_in;
        int ang, mean_angle, mean_count;
        int db_idx, in_idx;
        int Penalty_Knuckle_Count=0, Penalty_Knuckle_Score=0;
        int back_thres_db = db_back_min_x+(db_back_max_x-db_back_min_x)/3;
        int back_thres_in = in_back_min_x+(in_back_max_x-in_back_min_x)/3;

        mean_angle=0, mean_count=0;
        for(i=0; i<MatchedMinutiaCount; i++)
        {
            if(STEP_MATCEDMINUTIAE_SCORE_MINUTIAE[i] <= 0)
                continue;

            db_idx = STEP_MATCEDMINUTIAE_DB[i];
            in_idx = STEP_MATCEDMINUTIAE_IN[i];

            // 가로 영상이므로
            if(p_feavector_db->x[db_idx] > back_thres_db &&
                p_feavector_in->x[in_idx] > back_thres_in)
                continue;

            ang = p_feavector_db->angle[db_idx];
            if(ang > 180){ ang = 360 - ang; }

            // 90도를 기준으로 -45 ~ +15 사이의 angle이면 평균각 설정 가능한 minutia로 판단
            if(ang >= 45 && ang <= 105)
            {
                mean_angle+=ang;
                mean_count++;
            }
        }

        // 최소 3개 이상이면 Knuckle 계산 시작
        if(mean_count >= 3)
        {
            mean_angle /= mean_count;

            Penalty_Knuckle_Count=0, Penalty_Knuckle_Score=0;
            for(i=0; i<MatchedMinutiaCount; i++)
            {
                if(STEP_MATCEDMINUTIAE_SCORE_MINUTIAE[i] <= 0)
                    continue;

                db_idx = STEP_MATCEDMINUTIAE_DB[i];
                in_idx = STEP_MATCEDMINUTIAE_IN[i];

                // 가로 영상이므로
                if(p_feavector_db->x[db_idx] > back_thres_db &&
                    p_feavector_in->x[in_idx] > back_thres_in)
                    continue;

                ang = p_feavector_db->angle[db_idx];
                if(ang > 180){ ang = 360 - ang; }

                // 평균각과 -15 ~ 15 사이의 차이를 가진 minutia를 knuckle로 봄
                if(abs(mean_angle-ang) <= 15)
                {
                    Penalty_Knuckle_Count++;
                }
            }

            // 최소 2개 이상이면 Knuckle로 간주
            if(Penalty_Knuckle_Count >= 2)
            {
                Penalty_Knuckle_Score = Penalty_Knuckle_Count * 180;
                Total_NMS -= Penalty_Knuckle_Score;
            }
        }
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Penalty - 같은 방향 특징점 매칭 & 뭉쳐져 있음 & 공통영역이 큼
    {
        int ang, mean_angle, mean_count, db_idx;
        int Penalty_Same_Dir_Count=0, Penalty_Same_Dir_Score=0;
        int MatArea = (db_mat_max_x-db_mat_min_x)*(db_mat_max_y-db_mat_min_y) * 2;
        int CommMatArea = (db_comm_max_x-db_comm_min_x)*(db_comm_max_y-db_comm_min_y);

        mean_angle=0, mean_count=0;
        for(i=0; i<MatchedMinutiaCount; i++)
        {
            if(STEP_MATCEDMINUTIAE_SCORE_MINUTIAE[i] <= 0)
                continue;

            db_idx = STEP_MATCEDMINUTIAE_DB[i];
            ang = p_feavector_db->angle[db_idx];
            mean_angle+=ang;
            mean_count++;
        }
        if(mean_count > 0)
            mean_angle /= mean_count;

        for(i=0; i<MatchedMinutiaCount; i++)
        {
            if(STEP_MATCEDMINUTIAE_SCORE_MINUTIAE[i] <= 0)
                continue;

            db_idx = STEP_MATCEDMINUTIAE_DB[i];
            ang = p_feavector_db->angle[db_idx];

            // 평균각과 -20 ~ 20 사이의 차이를 가진 minutia가 대상
            ang = mean_angle - ang;
            if(ang < 0) ang += 360;
            if(ang > 180) ang = 360 - ang;
            if(ang <= 20)
                Penalty_Same_Dir_Count++;
        }

        if(MatchedMinutiaCount - Penalty_Same_Dir_Count <= 3 &&
            MatchedMinutiaCount >= 5 &&
            MatArea < CommMatArea)
        {
            Penalty_Same_Dir_Score = Penalty_Same_Dir_Count * 120;
            Total_NMS -= Penalty_Same_Dir_Score;
        }
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    return Total_NMS;
}

int CIBSMAlgorithm::_JudgeFingerPattern(const FeatureVector *ref_feature, const FeatureVector *inp_feature)
{
    int inp_coreType1, inp_coreType2, ref_coreType1, ref_coreType2;
    int inp_pitch, ref_pitch, inp_coreDist, ref_coreDist;
    int inp_quality, ref_quality;

    ref_quality = ref_feature->quality;
    ref_coreType1 = ref_feature->core_type>>4;
    ref_coreType2 = ref_feature->core_type&0xF;
    ref_pitch = ref_feature->pitch;
    ref_coreDist = ref_feature->core_dist;

    inp_quality = inp_feature->quality;
    inp_coreType1 = inp_feature->core_type>>4;
    inp_coreType2 = inp_feature->core_type&0xF;
    inp_pitch = inp_feature->pitch;
    inp_coreDist = inp_feature->core_dist;

    if( abs(ref_pitch - inp_pitch) > 2 && inp_pitch > 0 && ref_pitch > 0 && (ref_quality>=1 || inp_quality>=1) )
        return 0;

    m_MatchCoreLevel = 0;
    if( inp_coreType1 == ref_coreType1 || inp_coreType2 == ref_coreType2 )
        m_MatchCoreLevel += 2;

    if( inp_coreType1 == ref_coreType2 || inp_coreType2 == ref_coreType1 )
        m_MatchCoreLevel += 1;

    if( inp_coreType1 == D_FP_UNKNOWN_TYPE || ref_coreType1 == D_FP_UNKNOWN_TYPE )
        m_MatchCoreLevel = 3;

    if( m_IndexingLevel == 1 )
    {
        if( inp_coreType1 == D_FP_UNKNOWN_TYPE || ref_coreType1 == D_FP_UNKNOWN_TYPE )
        {
            return 1;
        }

        if( inp_coreType1 == ref_coreType1 || inp_coreType1 == ref_coreType2 ||
            inp_coreType2 == ref_coreType1 || inp_coreType2 == ref_coreType2 )
        {
            if(inp_coreType1 == D_FP_WHORL_TYPE && ref_coreType1 == D_FP_WHORL_TYPE && inp_coreDist != 0 && ref_coreDist != 0)
            {
                if(abs(ref_coreDist - inp_coreDist) > 35)
                {
                    return 0;
                }
            }

            if(((inp_coreType1 == D_FP_LEFT_LOOP_TYPE && ref_coreType1 == D_FP_LEFT_LOOP_TYPE && inp_coreType2 == D_FP_LEFT_LOOP_TYPE && ref_coreType2 == D_FP_LEFT_LOOP_TYPE) ||
                (inp_coreType1 == D_FP_RIGHT_LOOP_TYPE && ref_coreType1 == D_FP_RIGHT_LOOP_TYPE && inp_coreType2 == D_FP_RIGHT_LOOP_TYPE && ref_coreType2 == D_FP_RIGHT_LOOP_TYPE))
                && inp_coreDist != 0 && ref_coreDist != 0)
            {
                if(abs(ref_coreDist - inp_coreDist) > 15)
                {
                    return 0;
                }
            }

            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        if(inp_coreType1 == D_FP_WHORL_TYPE && ref_coreType1 == D_FP_WHORL_TYPE && inp_coreDist != 0 && ref_coreDist != 0)
        {
            if(abs(ref_coreDist - inp_coreDist) > 35)
            {
                return 0;
            }
        }

        if(((inp_coreType1 == D_FP_LEFT_LOOP_TYPE && ref_coreType1 == D_FP_LEFT_LOOP_TYPE && inp_coreType2 == D_FP_LEFT_LOOP_TYPE && ref_coreType2 == D_FP_LEFT_LOOP_TYPE) ||
            (inp_coreType1 == D_FP_RIGHT_LOOP_TYPE && ref_coreType1 == D_FP_RIGHT_LOOP_TYPE && inp_coreType2 == D_FP_RIGHT_LOOP_TYPE && ref_coreType2 == D_FP_RIGHT_LOOP_TYPE))
            && inp_coreDist != 0 && ref_coreDist != 0)
        {
            if(abs(ref_coreDist - inp_coreDist) > 15)
            {
                return 0;
            }
        }
    }

    return 1;
}
