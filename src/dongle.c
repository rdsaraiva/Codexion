/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: roda-fon <roda-fon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/04 14:00:26 by roda-fon          #+#    #+#             */
/*   Updated: 2026/11/09 12:00:00 by roda-fon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	join_queue(t_dongle *d, t_req req)
{
	pthread_mutex_lock(&d->lock);
	heap_push(&d->queue, req);
	pthread_mutex_unlock(&d->lock);
}

static void	grab(t_dongle *d, int id)
{
	pthread_mutex_lock(&d->lock);
	heap_remove(&d->queue, id);
	d->taken = 1;
	pthread_mutex_unlock(&d->lock);
}

static void	give_back(t_dongle *d, long free_at)
{
	pthread_mutex_lock(&d->lock);
	d->taken = 0;
	d->free_at = free_at;
	pthread_mutex_unlock(&d->lock);
}

int	dongles_take(t_coder *c)
{
	t_req	req;
	int		stopped;

	pthread_mutex_lock(&c->sim->table_lock);
	req.id = c->id;
	req.key = req_key(c);
	join_queue(c->first, req);
	join_queue(c->second, req);
	while (sim_running(c->sim) && !can_go(c))
		wait_turn(c->sim);
	stopped = !sim_running(c->sim);
	if (!stopped)
	{
		grab(c->first, c->id);
		grab(c->second, c->id);
	}
	pthread_mutex_unlock(&c->sim->table_lock);
	return (stopped);
}

void	dongles_release(t_coder *c)
{
	long	free_at;

	pthread_mutex_lock(&c->sim->table_lock);
	free_at = now_ms() + c->sim->p.cooldown;
	give_back(c->first, free_at);
	give_back(c->second, free_at);
	pthread_cond_broadcast(&c->sim->table_cond);
	pthread_mutex_unlock(&c->sim->table_lock);
}
