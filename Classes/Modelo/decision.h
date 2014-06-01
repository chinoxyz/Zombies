/*
 * decision.h
 *
 *  Created on: May 30, 2014
 *      Author: jose
 */

#ifndef DECISION_H_
#define DECISION_H_


#include "state.h"
#include "strategy.h"
#include <cassert>
#include <cstdio>
#include <iostream>
#include <string>
#include <map>
#include <algorithm>
class decision{
	/*
	 * Return a sparce vector with the sum of the distances of every element in se.
	 */
	map<int,int> getmap(set<position> se, map<position, int> dist){
		map<int, int> res;
		for(set<position>::iterator it = se.begin(); it != se.end(); it++){
			res[dist[*it]]++;
		}
		return res;
	}

	vector<position> stov(set<position> se){
		vector<position> res;
		for(set<position>::iterator it = se.begin(); it != se.end(); it++){
			res.push_back(*it);
		}
		return res;
	}

public:
	/*
	 * Return the position where the player should move after rolling the dice
	 */
	position movement(state &gs, int player = -1){
		if(player == -1) player = gs.getCurrentPlayer();
		position playerp = gs.world.playerVector[player];

		position res;
		map<position,int> dist, dor;
		map<int,int> dl,db,dz,dp, dh, dd;
		bool endCard;
		pair<vector<position>,vector<position> > pa;

		assert(gs.getCurrentPlayerInstance().isMachine());
		machine &us  = (machine &)gs.getCurrentPlayerInstance();

		endCard = gs.world.hasEndCard();


		pa = gs.getPossibleMoves();
		pa.first.push_back(playerp);

		moveStrategy st = us.getMoveStrategy();

		res = pa.first[0];
		double maxf = -1e9;
		double cf;

		dor =gs.world.getDistances(playerp);

		for(int i = 0; i < pa.first.size(); i++){
			/* Calculate distances */
			dp.clear(); dl.clear(); db.clear(); dz.clear(); dh.clear(); dd.clear();
			dist = gs.world.getDistances(pa.first[i]);
			for(int j = 0; j < gs.world.playerVector.size(); j++){
				if(j == gs.getCurrentPlayer()) continue;
				dp[dist[gs.world.playerVector[j]]]++;
			}
			dl = getmap(gs.world.lifeSet, dist);
			db = getmap(gs.world.bulletSet, dist);
			dz = getmap(gs.world.zombieSet, dist);
			if(endCard){
				dh[dist[gs.world.endPosition]]++;
			}
			dd[dor[pa.first[i]]]++;
			/* End Calculate distances */

			cf = st.f(dl, db, dz, dp, dh, dd);
			if(maxf < cf){
				res = pa.first[i];
				maxf = cf;
			}
		}
		return res;
	}

	/*
	 * Return the position where the player should put the last map card picked up
	 */
	pair<int,position> putmapcard(state &gs, int player = -1){
		if(player == -1) player = gs.getCurrentPlayer();
		position playerp = gs.world.playerVector[player];

		vector<vector<position> > ve = gs.getAllPosibleMapCard(gs.lastMapCard);
		position cur = playerp;
		position res;
		int resrot;
		if(gs.lastMapCard.isEndCard()){
			int mindist = 1e9;
			int dist;
			resrot = 0;
			for(int j = 0; j < ve[0].size(); j++){
				dist = cur.distManhattan(ve[0][j]);
				if(mindist > dist){
					res = ve[0][j];
					mindist = dist;
				}
			}

		}else{
			vector<int>vi;
			for(int i = 0; i < 4; i++) vi.push_back(i);
			random_shuffle(vi.begin(), vi.end());
			for(int i = 0; i < 4; i++){
				if(ve[vi[i]].empty()) continue;
				res = ve[vi[i]][rand()%ve[vi[i]].size()];
				resrot = i;
				break;
			}
		}
		return make_pair(resrot,res);
	}
	/*
	 * Return True if you must select life
	 * Return False if you must select bullet
	 */
	bool selectLife(state &gs){
		int lif,bul,dic;
		lif = gs.getCurrentPlayerLife();
		bul = gs.getCurrentPlayerBullet();
		dic = gs.getLastRollFightDice();
		if((lif == 0) || (dic == 3 && bul > 0)) return false;
		if((bul == 0) || (dic == 1 && lif > 0)) return true;
		return (rand()%2 == 0);
	}

	pair<position, position> moveZombie(state &gs, int player = -1){
		if(player == -1) player = gs.getCurrentPlayer();
		vector<position> ve = stov(gs.world.zombieSet);

	}


};




#endif /* DECISION_H_ */
