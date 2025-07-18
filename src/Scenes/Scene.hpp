/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Scene.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/17 10:38:02 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/18 10:46:43 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SCENE_HPP
# define SCENE_HPP

# include "libs.hpp"
# include "Camera.hpp"
# include "InterfaceManager.hpp"

extern Camera			*CAMERA;
extern InterfaceManager	*INTERFACE_MANAGER;

class	Scene
{
	public:
		Scene(std::function<void(Scene *)> build, std::function<void(Scene *)> destructor, std::function<void(Scene*)> onRender, std::function<void(Scene*)> onUpdate)
		{
			if (build == NULL || onRender == NULL || onUpdate == NULL || destructor == NULL)
				throw std::runtime_error("Scene base functions cannot be NULL");
			this->_destructor = destructor;
			this->_onRender = onRender;
			this->_onUpdate = onUpdate;
			_camera = new Camera();
			_interfaceManager = new InterfaceManager();
			build(this);
		}
		~Scene()
		{
			if (_destructor)
				_destructor(this);
			delete _camera;
			delete _interfaceManager;
		}
		void	use()
		{
			CAMERA = _camera;
			if (_onOpen)
				_onOpen(this);
		}
		void	close()
		{
			if (_onClose)
				_onClose(this);
		}
		void	render()
		{
			if (_onRender)
				_onRender(this);
		}
		void	update()
		{
			if (_onUpdate)
				_onUpdate(this);
		}
		void	keyHook(int key, int action)
		{
			if (_keyHook)
				_keyHook(this, key, action);
		}
		void	charHook(uint key)
		{
			if (_charHook)
				_charHook(this, key);
		}
		void	moveMouseHook(double xpos, double ypos)
		{
			if (_moveMouseHook)
				_moveMouseHook(this, xpos, ypos);
		}
		void	setKeyHook(std::function<void(Scene*, int, int)> keyHook) {this->_keyHook = keyHook;}
		void	setCharHook(std::function<void(Scene*, uint)> charHook) {this->_charHook = charHook;}
		void	setMoveMouseHook(std::function<void(Scene*, double, double)> moveMouseHook) {this->_moveMouseHook = moveMouseHook;}
		//Sets function called whenever the scene is closed (Switching to another scene)
		void	setClose(std::function<void(Scene*)> onClose) {this->_onClose = onClose;}
		//Sets function called whenever the scene is opened (Switching from another scene)
		void	setOpen(std::function<void(Scene*)> onOpen) {this->_onOpen = onOpen;}
		InterfaceManager	*getInterfaceManager() {return (this->_interfaceManager);}
		Camera				*getCamera() {return (this->_camera);}
	private:
		std::function<void(Scene*)>					_destructor = NULL; //Called whenever the scene gets destroyeds
		std::function<void(Scene*)>					_onRender = NULL; //Called to render a frame
		std::function<void(Scene*)>					_onUpdate = NULL; //Called every frame before rendering
		std::function<void(Scene*)>					_onOpen = NULL; //Called when the scene is set to use
		std::function<void(Scene*)>					_onClose = NULL; //Called when the scene is replaced by another
		std::function<void(Scene*, int, int)>		_keyHook = NULL; //int key, int action
		std::function<void(Scene*, uint)>			_charHook = NULL; //uint key
		std::function<void(Scene*, double, double)>	_moveMouseHook = NULL; //uint key
		Camera										*_camera = NULL;
		InterfaceManager							*_interfaceManager = NULL;
};

#endif
