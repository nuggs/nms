cmake_minimum_required(VERSION 2.8)

project(merc)

add_executable(merc act_comm.c act_info.c act_move.c act_obj.c act_wiz.c comm.c const.c
                    db.c fight.c handler.c interp.c magic.c protocol.c parson.c save.c
		    special.c update.c)

target_link_libraries(merc "crypt")

