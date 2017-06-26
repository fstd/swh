AUTOMAKE_OPTIONS = subdir-objects
SUBDIRS = common back front

bin_PROGRAMS = swh
swh_SOURCES = common/common.c common/common.h \
              common/log.c common/log.h \
              core.c core.h \
              sc.c sc.h \
              front/uc.c front/uc.h \
              spawn.c spawn.h \
              back/fsm.c back/fsm.h \
              nami.c nami.h \
              back/fsm_init.c back/fsm_init.h \
              back/fsm_cmdout.c back/fsm_cmdout.h \
              back/fsm_inchar.c back/fsm_inchar.h \
              back/ansiseq.c back/ansiseq.h \
              back/hp/common_hp.c back/hp/common_hp.h \
              back/hp/fsm_init_hp.c back/hp/fsm_init_hp.h \
              back/hp/fsm_cmdout_hp.c back/hp/fsm_cmdout_hp.h \
              back/hp/fsm_inchar_hp.c back/hp/fsm_inchar_hp.h \
              front/noop/uc_noop.c \
              front/ia/uc_ia.c \
              init.c
