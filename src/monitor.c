/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: roda-fon <roda-fon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/05 11:30:21 by roda-fon          #+#    #+#             */
/*   Updated: 2026/11/05 12:08:40 by roda-fon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	burned_out(t_sim *sim, t_coder *c)
{
	long	last;

	pthread_mutex_lock(&sim->state_lock);
	last = c->last_compile;
	pthread_mutex_unlock(&sim->state_lock);
	if (now_ms() - last <= sim->p.burnout)
		return (0);
	pthread_mutex_lock(&sim->print_lock);
	sim_stop(sim);
	printf("%ld %d burned out\n", now_ms() - sim->start, c->id);
	pthread_mutex_unlock(&sim->print_lock);
	return (1);
}

static int	all_done(t_sim *sim)
{
	int	i;
	int	done;

	done = 1;
	i = 0;
	pthread_mutex_lock(&sim->state_lock);
	while (i < sim->p.coders)
	{
		if (sim->coders[i].compiles < sim->p.required)
			done = 0;
		i++;
	}
	pthread_mutex_unlock(&sim->state_lock);
	return (done);
}

void	*monitor_routine(void *arg)
{
	t_sim	*sim;
	int		i;

	sim = arg;
	while (1)
	{
		i = 0;
		while (i < sim->p.coders)
		{
			if (burned_out(sim, &sim->coders[i]))
				return (NULL);
			i++;
		}
		if (all_done(sim))
		{
			sim_stop(sim);
			return (NULL);
		}
		usleep(1000);
	}
}
