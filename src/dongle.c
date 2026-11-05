/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: roda-fon <roda-fon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/04 14:00:26 by roda-fon          #+#    #+#             */
/*   Updated: 2026/11/04 15:00:00 by roda-fon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static long	req_key(t_coder *c, t_dongle *d)
{
	long	key;

	if (c->sim->p.sched == FIFO)
	{
		key = d->ticket;
		d->ticket++;
		return (key);
	}
	pthread_mutex_lock(&c->sim->state_lock);
	key = c->last_compile + c->sim->p.burnout;
	pthread_mutex_unlock(&c->sim->state_lock);
	return (key);
}

static int	can_take(t_dongle *d, int id)
{
	return (d->taken == 0
		&& d->queue.items[0].id == id
		&& now_ms() >= d->free_at);
}

static void	wait_turn(t_dongle *d)
{
	struct timeval	tv;
	struct timespec	ts;

	gettimeofday(&tv, NULL);
	ts.tv_sec = tv.tv_sec;
	ts.tv_nsec = tv.tv_usec * 1000 + 1000000;
	if (ts.tv_nsec >= 1000000000)
	{
		ts.tv_sec++;
		ts.tv_nsec -= 1000000000;
	}
	pthread_cond_timedwait(&d->cond, &d->lock, &ts);
}

int	dongle_take(t_coder *c, t_dongle *d)
{
	t_req	req;

	pthread_mutex_lock(&d->lock);
	req.id = c->id;
	req.key = req_key(c, d);
	heap_push(&d->queue, req);
	while (sim_running(c->sim) && !can_take(d, c->id))
		wait_turn(d);
	if (!sim_running(c->sim))
	{
		pthread_mutex_unlock(&d->lock);
		return (1);
	}
	heap_pop(&d->queue);
	d->taken = 1;
	pthread_mutex_unlock(&d->lock);
	return (0);
}

void	dongle_release(t_coder *c, t_dongle *d)
{
	pthread_mutex_lock(&d->lock);
	d->taken = 0;
	d->free_at = now_ms() + c->sim->p.cooldown;
	pthread_cond_broadcast(&d->cond);
	pthread_mutex_unlock(&d->lock);
}
