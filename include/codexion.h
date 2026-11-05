/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: roda-fon <roda-fon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/26 12:00:57 by roda-fon          #+#    #+#             */
/*   Updated: 2026/11/05 11:30:01 by roda-fon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <stdio.h>
# include <string.h>
# include <limits.h>
# include <sys/time.h>
# include <unistd.h>
# include <stdlib.h>
# include <pthread.h>

typedef enum e_sched
{
	FIFO,
	EDF
}	t_sched;

typedef struct s_params
{
	int		coders;
	long	burnout;
	long	compile;
	long	debug;
	long	refactor;
	int		required;
	long	cooldown;
	t_sched	sched;
}	t_params;

typedef struct s_req
{
	int		id;
	long	key;
}	t_req;

typedef struct s_heap
{
	t_req	*items;
	int		size;
	int		capacity;
}	t_heap;

typedef struct s_dongle
{
	pthread_mutex_t	lock;
	pthread_cond_t	cond;
	int				taken;
	long			free_at;
	long			ticket;
	t_heap			queue;
}	t_dongle;

typedef struct s_coder
{
	int				id;
	pthread_t		thread;
	long			last_compile;
	int				compiles;
	t_dongle		*first;
	t_dongle		*second;
	struct s_sim	*sim;
}	t_coder;

typedef struct s_sim
{
	t_params		p;
	long			start;
	int				stop;
	pthread_mutex_t	state_lock;
	pthread_mutex_t	print_lock;
	t_coder			*coders;
	t_dongle		*dongles;
	pthread_t		monitor;
}	t_sim;

/* parse.c */
int		parse_args(int argc, char **argv, t_params *p);

/* time.c */
long	now_ms(void);
void	sleep_ms(long ms, t_sim *sim);

/* heap_utils.c */
int		req_less(t_req a, t_req b);
void	heap_swap(t_req *a, t_req *b);
void	sift_up(t_heap *h, int i);
void	sift_down(t_heap *h, int i);

/* heap.c */
int		heap_init(t_heap *h, int capacity);
void	heap_free(t_heap *h);
int		heap_push(t_heap *h, t_req req);
t_req	heap_pop(t_heap *h);

/* state.c */
int		sim_running(t_sim *sim);
void	sim_stop(t_sim *sim);
void	log_state(t_sim *sim, int id, char *msg);

/* init.c */
int		sim_init(t_sim *sim);

/* cleanup.c */
void	sim_destroy(t_sim *sim);

/* dongle.c */
int		dongle_take(t_coder *c, t_dongle *d);
void	dongle_release(t_coder *c, t_dongle *d);

/* coder.c */
void	*coder_routine(void *arg);

/* monitor.c */
void	*monitor_routine(void *arg);

#endif