import React from 'react';
import classnames from 'classnames';
import { NavLink } from 'react-router-dom';

import RoundButton from './buttons/RoundButton.component';

import './Header.styles.scss';

export default class Header extends React.Component<any, any>
{
  state: {
    menu: {
      open: boolean
    }
  };

  public constructor(props: any)
  {
    super(props);

    this.state = {
      menu: {
        open: true
      }
    };
  }

  /**
   * Toggles the menu
   * 
   * @param {Event} e
   */
  onMenuToggle = (e: Event): void =>
  {
    e.preventDefault();
    const { menu } = this.state;

    this.setState({
      menu: Object.assign(menu, {
        open: !menu.open
      })
    });
  }

  public render(): any
  {
    const { menu } = this.state;

    const navClassnames = classnames({
      'nav-state__idle': !menu.open,
      'nav-state__active': menu.open,
      'nav': true
    });

    return (
      <React.Fragment>
        <div className="nav__fixed-frag">
          <header className="header">
            {/* Logo and menu button */}
            <div className="header__logo">
              <div>
                <RoundButton onClick={this.onMenuToggle}>
                  {menu.open ? (
                    <svg
                      xmlns="http://www.w3.org/2000/svg"
                      height="24"
                      viewBox="0 0 24 24"
                      width="24"
                    >
                      <path d="M0 0h24v24H0V0z" fill="none"/>
                      <path d="M3 18h13v-2H3v2zm0-5h10v-2H3v2zm0-7v2h13V6H3zm18 9.59L17.42 12 21 8.41 19.59 7l-5 5 5 5L21 15.59z"/>
                    </svg>
                  ) : (
                    <svg 
                      xmlns="http://www.w3.org/2000/svg"
                      height="24"
                      viewBox="0 0 24 24"
                      width="24"
                    >
                      <path d="M0 0h24v24H0z" fill="none"/>
                      <path d="M3 18h18v-2H3v2zm0-5h18v-2H3v2zm0-7v2h18V6H3z"/>
                    </svg>
                  )}
                </RoundButton>
              </div>
              <div>
                <h1>FSMTP-V2</h1>
                <p>By Fannst Software</p>
              </div>
            </div>
            {/* Search bar */}
            <div className="header__search">
              <form className="header__search-form">
                <input type="text" placeholder="Search ..." />
                <button type="submit">
                  <svg
                    xmlns="http://www.w3.org/2000/svg"
                    height="24"
                    viewBox="0 0 24 24"
                    width="24"
                  >
                    <path d="M0 0h24v24H0z" fill="none"/>
                    <path d="M20 19.59V8l-6-6H6c-1.1 0-1.99.9-1.99 2L4 20c0 1.1.89 2 1.99 2H18c.45 0 .85-.15 1.19-.4l-4.43-4.43c-.8.52-1.74.83-2.76.83-2.76 0-5-2.24-5-5s2.24-5 5-5 5 2.24 5 5c0 1.02-.31 1.96-.83 2.75L20 19.59zM9 13c0 1.66 1.34 3 3 3s3-1.34 3-3-1.34-3-3-3-3 1.34-3 3z"/>
                  </svg>
                </button>
              </form>
            </div>
          </header>
          <nav className={navClassnames}>
            <div className="nav__folders">
              <label htmlFor="ulNavFolders">Folders: </label>
              <ul className="nav__folders-ul" id="ulNavFolders">
                <li>
                  <NavLink
                    activeClassName="nav__folders-ul__li__a-active"
                    className="nav__folders-ul__li__a"
                    to="/inbox"
                  >Inbox</NavLink>
                </li>
                <li>
                  <NavLink
                    activeClassName="nav__folders-ul__li__a-active"
                    className="nav__folders-ul__li__a"
                    to="/sent"
                  >Sent</NavLink>
                </li>
                <li>
                  <NavLink
                    activeClassName="nav__folders-ul__li__a-active"
                    className="nav__folders-ul__li__a"
                    to="/trash"
                  >Trashbin</NavLink>
                </li>
                <li>
                  <NavLink
                    activeClassName="nav__folders-ul__li__a-active"
                    className="nav__folders-ul__li__a"
                    to="/spam"
                  >Spam</NavLink>
                </li>
              </ul>
            </div>
            <div className="nav__options">

            </div>
          </nav>
        </div>
      </React.Fragment>
    );
  }
}