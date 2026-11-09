/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: roda-fon <roda-fon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/09 12:00:00 by roda-fon          #+#    #+#             */
/*   Updated: 2026/11/09 12:00:00 by roda-fon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long	req_key(t_coder *c)
{
	long	key;

	if (c->sim->p.sched == FIFO)
	{
		key = c->sim->ticket;
		c->sim->ticket++;
		return (key);
	}
	pthread_mutex_lock(&c->sim->state_lock);
	key = c->last_compile + c->sim->p.burnout;
	pthread_mutex_unlock(&c->sim->state_lock);
	return (key);
}

static t_coder	*top_of(t_sim *sim, t_dongle *d)
{
	if (d->queue.size == 0)
		return (NULL);
	return (&sim->coders[d->queue.items[0].id - 1]);
}

static int	blocked_by(t_sim *sim, t_dongle *d, t_coder *c)
{
	t_coder	*top;

	top = top_of(sim, d);
	if (top == NULL || top == c)
		return (0);
	return (top_of(sim, top->first) == top
		&& top_of(sim, top->second) == top);
}

int	can_go(t_coder *c)
{
	long	now;

	now = now_ms();
	return (c->first->taken == 0 && now >= c->first->free_at
		&& c->second->taken == 0 && now >= c->second->free_at
		&& !blocked_by(c->sim, c->first, c)
		&& !blocked_by(c->sim, c->second, c));
}

void	wait_turn(t_sim *sim)
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
	pthread_cond_timedwait(&sim->table_cond, &sim->table_lock, &ts);
}
