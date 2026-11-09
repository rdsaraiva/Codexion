/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: roda-fon <roda-fon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/04 17:49:37 by roda-fon          #+#    #+#             */
/*   Updated: 2026/11/09 12:00:00 by roda-fon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	lonely_coder(t_coder *c)
{
	log_state(c->sim, c->id, "has taken a dongle");
	while (sim_running(c->sim))
		usleep(500);
	return (1);
}

static int	compile_once(t_coder *c)
{
	if (c->first == c->second)
		return (lonely_coder(c));
	if (dongles_take(c))
		return (1);
	pthread_mutex_lock(&c->sim->state_lock);
	c->last_compile = now_ms();
	pthread_mutex_unlock(&c->sim->state_lock);
	log_compile(c->sim, c->id);
	sleep_ms(c->sim->p.compile, c->sim);
	pthread_mutex_lock(&c->sim->state_lock);
	c->compiles++;
	pthread_mutex_unlock(&c->sim->state_lock);
	dongles_release(c);
	return (0);
}

void	*coder_routine(void *arg)
{
	t_coder	*c;

	c = arg;
	while (sim_running(c->sim))
	{
		if (compile_once(c))
			break ;
		log_state(c->sim, c->id, "is debugging");
		sleep_ms(c->sim->p.debug, c->sim);
		log_state(c->sim, c->id, "is refactoring");
		sleep_ms(c->sim->p.refactor, c->sim);
	}
	return (NULL);
}
